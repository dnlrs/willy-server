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
    clear_state();
}

void receiver::start()
{
    /* this assures a clean start state */
    clear_state();

    raw_packets_queue = std::make_shared<sync_queue>();
    packet_collector  = std::make_shared<collector>(context);

    int workers_nr = get_workers_number();

    debuglog("deploying " + std::to_string(workers_nr) + " workers\n");
    for (int i = 0; i < workers_nr; i++) {
        std::shared_ptr<worker> new_worker = 
            std::make_shared<worker>(
                context, raw_packets_queue, packet_collector);
        new_worker->start();
        workers.push_back(new_worker);
    }
    
    stop_working    = false;
    receiver_thread = std::thread(&receiver::service, this);
}

void receiver::stop()
{
    stop_working = true;

    for (std::shared_ptr<worker> employee : workers)
        employee->stop();
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
}

void receiver::clear_state()
{
    stop();
    finish();
}

void
receiver::service()
{
    std::vector<SOCKET> active_sockets;
    
    // select timeout structure
    struct timeval tm;
    tm.tv_sec  = select_timeout_sec;
    tm.tv_usec = select_timeout_usec;
    
    // select returned ready sockets
    int n = 0;
    
    fd_set active_rset;

    while (stop_working == false) {
        active_sockets = broker.get_opened_sockets();

        if (active_sockets.size() == 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(default_sleep_ms));
            continue; // no opened sockets exists
        }
    
        FD_ZERO(&active_rset);

        for (SOCKET sock : active_sockets)
            FD_SET(sock, &active_rset);

        n = ::select(FD_SETSIZE, &active_rset, NULL, NULL, &tm);
            
        if (n == 0) {
            // select timeout expired
            continue; 
        }

        if (n == SOCKET_ERROR) {
            /* select failed: this is a fatal error and under 
             * normal conditions should not happen
             **/
            debuglog("receiver: select failed\n" + wsa_etos(WSAGetLastError()));
            broker.notify_fatal_error();
            stop_working = true;
            continue;
        }

        /* read from each ready socket and store messages in the 
         * shared fifo queue 
         **/
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
        } catch (sock_exception& sock_ex) {
            broker.notify_anchor_disconnected(sock_ex.get_socket());
        } catch (net_exception& net_ex) {
            broker.notify_fatal_error();
        }
    }
}

int
receiver::get_workers_number()
{
    int hw_concurrency = std::thread::hardware_concurrency();
 
    if (hw_concurrency == 0)
        return default_workers_number;

    hw_concurrency -= 2;

    if (hw_concurrency < default_workers_number)
        return default_workers_number;


    return hw_concurrency;
}