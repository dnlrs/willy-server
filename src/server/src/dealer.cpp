#include "dealer.h"
#include "utils.h"
#include "socket_utils.h"
#include "wARPtable.h"
#include <ws2tcpip.h>
#include <mstcpip.h>

dealer::dealer()
{
    listening_socket = INVALID_SOCKET;
    stop_working = false;
    connections  = nullptr;
    dead_anchors = nullptr;
    preceiver    = nullptr;
    context      = nullptr;
}

dealer::~dealer()
{
    shutdown();
    if (listening_socket != INVALID_SOCKET)
        closesocket(listening_socket);
}

void dealer::init(std::string conf_file)
{
    // avoid changing configuration while the server is running
    if (preceiver != nullptr)
        shutdown();

    // new configuration
    context = std::make_shared<cfg::configuration>();

    // read configuration
    context->import_configuration(conf_file, true);
    if (context->get_anchors_number() == 0)
        throw net_exception("dealer::init: "
            "no anchors found in configuration file");
    debuglog("dealer::init: importing configuration... OK");

    // avoid trying to open two listneing sockets
    if (listening_socket != INVALID_SOCKET)
        closesocket(listening_socket);

    // setup listening socket
    listening_socket = setup_listening_socket(SERVICE_PORT);
    debuglog("dealer::init: listening socket setup... OK");
}

void dealer::start()
{
    // this assures a clean start
    if (preceiver != nullptr)
        shutdown();

    // new connections storage
    connections = std::make_shared<connection_set>();

    // start receiver
    dead_anchors =
        std::make_shared<std::atomic_int>(context->get_anchors_number());
    preceiver = 
        std::make_unique<receiver>(*this, context, connections, dead_anchors);
    preceiver->start();

    // start dealer's thread
    stop_working  = false;
    dealer_thread = std::thread(&dealer::service, this);
    debuglog("dealer::start: dealer thread started... OK");
}

void dealer::stop()
{
    if (preceiver != nullptr)
        preceiver->stop();

    stop_working = true;
    debuglog("dealer::stop: sending stop request... OK");
}

void dealer::finish()
{
    if (preceiver != nullptr)
        preceiver->finish();

    if (dealer_thread.joinable())
        dealer_thread.join();

    preceiver    = nullptr;
    dead_anchors = nullptr;

    connections->close_all_connections();
    connections = nullptr;
    debuglog("dealer::finish: stopping dealer thread... OK");
}

void dealer::shutdown()
{
    stop();
    finish();
}

void
dealer::service()
{
    debuglog("dealer thread:", std::this_thread::get_id());
    debuglog("dealer::service: "
        "waiting for ", dead_anchors->load(), " anchors to connect...");

    std::unique_lock<std::mutex> guard(dealer_mtx);
    while (stop_working == false) {
        if (
            dealer_cv.wait_for(
                guard,
                std::chrono::milliseconds(dealer_waiting_time_ms),
                [this]() -> bool { return (dead_anchors->load() > 0); })
            ) {
            /* This condition is true when the lambda expression is true;
             *   When the timeout expires the condition fis false and
             * the execution flow wraps around the cycle checking the
             * thread stopping condition.
             *   Spurious notification are avoided using the lambda
             * expression.
             */
            SOCKET new_socket = INVALID_SOCKET;
            anchor new_anchor = connect_anchor(&new_socket);

            if (new_socket == INVALID_SOCKET) {
                // no anchor connected
                continue;
            }

            // get position of newly connected anchor
            std::pair<double, double> new_anchor_position(0, 0);
            bool rs = context->get_anchor_position(
                new_anchor.get_mac(), new_anchor_position);

            if (rs == true) {
                // newly connected anchor found in configuration
                new_anchor.set_position(new_anchor_position);

                connections->add_connected_anchor(new_socket, new_anchor);

                dead_anchors->fetch_sub(1);
                debuglog("dealer::service: "
                    "anchor connected: ", new_anchor.str());
            }
            else {
                // newly connected anchor not found in configuration
                debuglog("dealer::service: "
                    "unknown anchor " + new_anchor.str(), " closing connection");
                close_connection(&new_socket);
            }
        }
    }
}

anchor
dealer::connect_anchor(SOCKET* rsocket)
{
    if (listening_socket == INVALID_SOCKET)
        throw net_exception(
            "Cannot connect anchor because listening socket is invalid");

    *rsocket = INVALID_SOCKET;

    // accept new connection
    struct sockaddr_in anchor_sa;
    size_t anchor_sa_size = sizeof(anchor_sa);
    memset(&anchor_sa, 0, anchor_sa_size);

    int    attempts = dealer_max_accept_attempts;
    SOCKET new_socket = INVALID_SOCKET;
    while (attempts > 0 && new_socket == INVALID_SOCKET) {
        new_socket =
            ::accept(listening_socket, (sockaddr*)&anchor_sa, (int*)&anchor_sa_size);

        if (new_socket == INVALID_SOCKET) {
            int wsa_err = WSAGetLastError();

            switch (wsa_err) {
            case WSAEWOULDBLOCK:
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(dealer_waiting_time_ms));
                attempts--;
                break;
            case WSAECONNRESET:
                attempts = dealer_max_accept_attempts;
                break;
            default:
                throw net_exception("Accept failed while connecting an anchor\n" +
                    wsa_etos(WSAGetLastError()));
                break;
            }
        }
    }

    if (attempts <= 0) {
        *rsocket = INVALID_SOCKET;
        return anchor();
    }

    // get anchor mac
    wARPtable arp_table;
    ip_addr  new_ip(anchor_sa.sin_addr.s_addr);
    mac_addr new_mac = arp_table.get_mac_from_ip(new_ip);

    set_keepalive_option(new_socket);
    set_non_blocking_socket(new_socket);

    send_connection_ack(new_socket);

    // return new socket and new anchor
    *rsocket = new_socket;
    return anchor(new_mac, new_ip);
}

void
dealer::send_connection_ack(const SOCKET anchor_socket)
{
    if (anchor_socket == INVALID_SOCKET)
        throw net_exception("Cannot send connection ack on invalid socket");

    uint32_t ack = 1;
    uint32_t left_bytes = sizeof(ack);
    const char *pbuf = (const char *)&ack;

    while (left_bytes > 0) {
        uint32_t sent_bytes =
            ::send(anchor_socket, pbuf, left_bytes, 0);

        if (sent_bytes == SOCKET_ERROR)
            throw net_exception("Failed to send connection ACK\n" +
                wsa_etos(WSAGetLastError()));

        pbuf += sent_bytes;
        left_bytes -= sent_bytes;
    }
}

void
dealer::notify_anchor_disconnected(SOCKET dead_socket)
/*
 * Close the connection, remove anchor and wake up
 * dealer's thread.
 */
{
    connections->close_connection(dead_socket);
    dead_anchors->fetch_add(1);
    dealer_cv.notify_all();
}

void
dealer::notify_fatal_error()
/*
 * Do NOT call shutdown(), otherwise deadlock!
 */
{
    debuglog("FATAL ERROR: fatal error notified, stopping dealer.\n");
    stop();
}
