#include "dealer.h"
#include "net_exception.h"
#include "receiver.h"
#include "recv_exception.h"
#include "sized_buffer.hpp"
#include "socket_utils.h"
#include "sock_exception.h"
#include "utils.h"
#include <chrono>
#include <thread>

receiver::receiver(
    dealer& dealer_ref,
    std::shared_ptr<cfg::configuration> context_in,
    std::shared_ptr<std::atomic_int>    dead_anchors_in) :
        broker(dealer_ref),
        context(context_in),
        dead_anchors(dead_anchors_in)
{
    stop_working = false;
    raw_packets_queue = nullptr;
    packet_collector  = nullptr;
}

receiver::~receiver()
{
    if (raw_packets_queue != nullptr)
        shutdown_receiver();
}

void receiver::start()
{
    // this assures a clean start
    if (raw_packets_queue != nullptr)
        shutdown_receiver();

    raw_packets_queue = std::make_shared<sync_queue>();
    packet_collector  = std::make_shared<collector>(context);

    int workers_nr = get_workers_number();
    debuglog("receiver::start: "
        "deploying " + std::to_string(workers_nr) + " workers\n");
    for (int i = 0; i < workers_nr; i++) {
        std::shared_ptr<worker> new_worker =
            std::make_shared<worker>(
                context, raw_packets_queue, packet_collector);
        new_worker->start();
        workers.push_back(new_worker);
    }

    stop_working    = false;
    receiver_thread = std::thread(&receiver::service, this);
    debuglog("receiver::start: receiver thread started... OK");
}

void receiver::stop()
{
    stop_working = true;

    for (std::shared_ptr<worker> employee : workers)
        employee->stop();
    debuglog("receiver::start: sending stop requst... OK");
}

void receiver::finish()
{
    for (std::shared_ptr<worker> employee : workers)
        employee->finish();
    workers.clear();

    if (receiver_thread.joinable())
        receiver_thread.join();

    raw_packets_queue = nullptr;
    packet_collector  = nullptr;
    debuglog("receiver::finish: stopping receiver thread... OK");
}

void receiver::shutdown_receiver()
{
    stop();
    finish();
}

void
receiver::service()
{
    debuglog("receiver thread:", std::this_thread::get_id());
    std::vector<SOCKET> active_sockets;

    // select timeout structure
    struct timeval tm;
    tm.tv_sec  = receiver_select_timeout_sec;
    tm.tv_usec = receiver_select_timeout_usec;

    // select returned ready sockets
    int ready_sockets_nr = 0;

    fd_set active_rset;

    while (stop_working == false) {
        active_sockets = broker.get_opened_sockets();

        if (active_sockets.size() == 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(receiver_sleeping_time_ms));
            continue; // no opened sockets exists
        }

        FD_ZERO(&active_rset);

        for (SOCKET sock : active_sockets)
            FD_SET(sock, &active_rset);

        ready_sockets_nr =
            ::select(FD_SETSIZE, &active_rset, NULL, NULL, &tm);

        if (ready_sockets_nr == 0) {
            // select timeout expired
            continue;
        }

        if (ready_sockets_nr == SOCKET_ERROR) {
            // select failed, should not happen under normal conditions
            debuglog("receiver::service: "
                "select failed\n" + wsa_etos(WSAGetLastError()));
            broker.notify_fatal_error();
            stop_working = true;
            continue;
        }

        // empty ready sockets' buffers
        try {
            for (SOCKET sock : active_sockets) {
                if (FD_ISSET(sock, &active_rset)) {
                    sized_buffer buffer;
                    buffer.msg_size   = read_sized_message(buffer.msg, sock);
                    buffer.anchor_mac = broker.get_anchor_mac(sock);

                    if (dead_anchors->load() == 0) {
                        raw_packets_queue->push(buffer);
                    }
                }
            }
        }
        catch (sock_exception& sock_ex) {
            debuglog("receiver::service: got socket exception");
            broker.notify_anchor_disconnected(sock_ex.get_socket());
        }
        catch (net_exception& net_ex) {
            debuglog("receiver::service: got network exception");
            broker.notify_fatal_error();
        }
    }
}

int
receiver::get_workers_number()
{
    int hw_concurrency = std::thread::hardware_concurrency();

    if (hw_concurrency == 0)
        return receiver_min_workers_number;

    hw_concurrency -= 2;

    if (hw_concurrency < receiver_min_workers_number)
        return receiver_min_workers_number;

    return hw_concurrency;
}
