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
    dead_anchors = nullptr;
    preceiver    = nullptr;
    context      = nullptr;
}

dealer::~dealer()
{
    clear_state();
    closesocket(listening_socket);
}


void dealer::init(std::string conf_file)
{
    // new configuration
    context = std::make_shared<cfg::configuration>();

    // read configuration
    context->import_configuration(conf_file, true);
    if (context->get_anchors_number() == 0)
        throw net_exception("No anchors found in configuration file");
    debuglog("dealer::init: configuration imported correctly");

    // setup listening socket
    setup_listening_socket();
}


void dealer::start()
{
    /* this assures a clean start */
    clear_state();

    dead_anchors = 
        std::make_shared<std::atomic_int>((int)context->get_anchors_number());
    
    preceiver = std::make_unique<receiver>(*this, context, dead_anchors);
    preceiver->start();

    stop_working  = false;
    dealer_thread = std::thread(&dealer::service, this);
}


void dealer::stop()
{   
    if (preceiver != nullptr)
        preceiver->stop();

    stop_working = true;
}

void dealer::finish()
{
    if (preceiver != nullptr)
        preceiver->finish();
    
    if (dealer_thread.joinable())
        dealer_thread.join();

    preceiver    = nullptr;
    dead_anchors = nullptr;
}

void dealer::clear_state()
{
    stop();
    finish();
    close_all_connections();
}

void
dealer::service()
{
    debuglog("dealer::service: dealer started");
    debuglog("Waiting for ", dead_anchors->load(), " anchors to connect...");

    std::unique_lock<std::mutex> guard(anchors_rmtx);
    while (stop_working == false) {

        /* This condition variable will return true if the lambda expression
         * is true;
         * will return false in all other cases:
         * - timeout expired
         * - spurious notification 
         * */
        if (dealer_cv.wait_for(
                guard, 
                std::chrono::milliseconds(dealer_waiting_time_ms),
                [this] () -> bool {
                    return dead_anchors->load() > 0 ? true : false;
                })) {

            SOCKET new_socket = INVALID_SOCKET;
            anchor new_anchor = connect_anchor(&new_socket);

            if (new_socket == INVALID_SOCKET) {
                debuglog(
                    "dealer::service: no anchor connected"
                    "after all attempts, trying again...");
                continue;
            }

            std::pair<double, double> new_anchor_position(0, 0);
            bool rs = context->get_anchor_position(
                        new_anchor.get_mac(), 
                        new_anchor_position);
            
            /* If anchor was found in the system configuration then add
             * the new anchor to the system and notify the receiver thread;
             * otherwise log unrecognized anchor and close connection
             * */
            if (rs == true) {
                new_anchor.set_position(new_anchor_position);
                add_connected_anchor(new_socket, new_anchor);

                dead_anchors->fetch_sub(1);
                debuglog("New anchor connected: ", new_anchor.to_string());
            }
            else {
                debuglog("Anchor " + mac_int2str(new_anchor.get_mac()) +
                    " was not found in configuration file");
                close_connection(&new_socket);
            }
        }
    }
    debuglog("dealer::service: dealer stopped");
}

void dealer::setup_listening_socket()
{
    if (listening_socket != INVALID_SOCKET)
        throw net_exception("listening socket should be invalid");

	// Create a SOCKET for listening for incoming connection requests
	listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listening_socket == INVALID_SOCKET) 
        throw net_exception(
            "listening_socket creation fail\n" + wsa_etos(WSAGetLastError()));

    // make listening socket non-blocking
    set_non_blocking_socket(listening_socket);
	
	// local endpoint parameters for listening socket (addr. family, IP, port)
	struct sockaddr_in service;
    memset(&service, 0, sizeof(service));

	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port        = htons(SERVICE_PORT);
	service.sin_family      = AF_INET;

    // Bind socket
    int err = SOCKET_ERROR;
    err = ::bind(listening_socket, (const sockaddr*) &service, (int) sizeof(service));
	if (err == SOCKET_ERROR) {
        closesocket(listening_socket);
		listening_socket = INVALID_SOCKET;
        throw net_exception(
            "listening_socket bind fail\n" + wsa_etos(WSAGetLastError()));
	}
	
	// Listen for incoming connection requests
    err = ::listen(listening_socket, SOMAXCONN);
	if (err == SOCKET_ERROR) {
		closesocket(listening_socket);
		listening_socket = INVALID_SOCKET;
        throw net_exception(
            "listening_socket liste fail\n" + wsa_etos(WSAGetLastError()));
	}
}

void 
dealer::add_connected_anchor(
    const SOCKET new_socket, 
    const anchor new_anchor)
{
    if (new_socket == INVALID_SOCKET)
        throw net_exception(
            "Cannot add a new connected anchor with invalid socket");
    
    uint64_t anchor_mac = new_anchor.get_mac();
    
    if (anchors.find(anchor_mac) != anchors.end()) {
        debuglog("Newly connected anchor was connected previously");
        remove_connected_anchor(anchor_mac);
    }

    anchors[anchor_mac]       = new_anchor;
    mac_to_socket[anchor_mac] = new_socket;
    socket_to_mac[new_socket] = anchor_mac;
}

void
dealer::remove_connected_anchor(
    const uint64_t anchor_mac)
{
    SOCKET old_socket = mac_to_socket[anchor_mac];
    
    if (anchors.find(anchor_mac) != anchors.end())
        anchors.erase(anchor_mac);
    
    if (mac_to_socket.find(anchor_mac) != mac_to_socket.end())
        mac_to_socket.erase(anchor_mac);
    
    if (socket_to_mac.find(old_socket) != socket_to_mac.end())
        socket_to_mac.erase(old_socket);

    close_connection(&old_socket);
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

    int    attempts   = dealer_max_accept_attempts;
    SOCKET new_socket = INVALID_SOCKET;
    while (attempts > 0 && new_socket == INVALID_SOCKET) {
        new_socket = 
            ::accept(listening_socket, (sockaddr*) &anchor_sa, (int*) &anchor_sa_size);
        
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
    uint64_t new_mac = arp_table.get_mac_from_ip(anchor_sa.sin_addr.s_addr);

    set_keepalive_option(new_socket);
    set_non_blocking_socket(new_socket);

    send_connection_ack(new_socket);

    // return new socket and new anchor
    *rsocket = new_socket;
    return anchor(new_mac, anchor_sa.sin_addr.s_addr);
}


void dealer::close_all_connections()
{
    std::unique_lock<std::mutex> guard(anchors_rmtx);

    for (auto open_connection : socket_to_mac) {
        SOCKET   open_socket = open_connection.first;
        uint64_t anchor_mac  = open_connection.second;

        close_connection(&open_socket);
        remove_connected_anchor(anchor_mac);
    }
}



void 
dealer::send_connection_ack(const SOCKET anchor_socket)
{
    if (anchor_socket == INVALID_SOCKET)
        throw net_exception("Cannot send connection ack on invalid socket");

    uint32_t ack = 1;
    uint32_t left_bytes = sizeof(ack);
    const char *pbuf = (const char *) &ack;
    
    while (left_bytes > 0) {
        uint32_t sent_bytes = 
            ::send(anchor_socket, pbuf, left_bytes, 0);

        if (sent_bytes == SOCKET_ERROR)
            throw net_exception("Failed to send connection ACK\n" + 
                                wsa_etos(WSAGetLastError()));

        pbuf       += sent_bytes;
        left_bytes -= sent_bytes;
    }
}

uint64_t
dealer::get_anchor_mac(SOCKET in_socket)
{
    std::lock_guard<std::mutex> guard(anchors_rmtx);
    // TODO: what if wrong socket in?
    return socket_to_mac[in_socket];
}

std::vector<SOCKET> 
dealer::get_opened_sockets()
{
    std::vector<SOCKET> rval;

    std::lock_guard<std::mutex> guard(anchors_rmtx);

    for (auto soc_pair : socket_to_mac)
        rval.push_back(soc_pair.first);

    return rval;
}

void
dealer::notify_anchor_disconnected(SOCKET dead_socket)
{
    dead_anchors->fetch_add(1);
    dealer_cv.notify_all();
}

void
dealer::notify_fatal_error()
{
    debuglog("FATAL ERROR: fatal error notified, stopping dealer.\n");
    stop();
}
