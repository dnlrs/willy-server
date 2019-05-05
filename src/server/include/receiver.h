#ifndef RECEIVER_H_INCLUDED
#define RECEIVER_H_INCLUDED
#pragma once

#include "cfg.h"
#include "collector.h"
#include "connection_set.h"
#include "sync_queue.h"
#include "worker.h"
#include <thread>
#include <atomic>

constexpr long int receiver_sleeping_time_ms    = 200;
constexpr long int receiver_select_timeout_sec  = 0;
constexpr long int receiver_select_timeout_usec = 20000;

constexpr int receiver_min_workers_number = 4;

// forward declaration
class dealer;

class receiver
{
public:
    receiver(
        dealer& dealer_ref,
        std::shared_ptr<cfg::configuration> context_in,
        std::shared_ptr<connection_set>     connections_in,
        std::shared_ptr<std::atomic_int>    dead_anchors_in);

    ~receiver();

    /* Asks the workers to start and deploys the receiver thread */
    void start();

    /* Asks workers to stop and stops the receiver thread */
    void stop();

    /* Asks workers to finish and joins the receiver thread */
    void finish();

private:
    /* Strategy:
     *   While all anchors are connected read data from the sockets
     * and push buffers into the chared queue for deserialization.
     * If an anchor disconnects, continue to read data from sockets
     * (in order to empty socket buffers) but ignore it.
     */
    void service();

    /* Calls stop() then finish() (blocking) */
    void shutdown_receiver();

    /* Gets the hardware concurrency capability */
    int get_workers_number();

private:
    /* reference to dealer */
    dealer& broker;

    /* Data about connected anchors and connections 
     * Only part of the information is accessible to the 
     * receiver */
    std::shared_ptr<connection_set> connections = nullptr;

    /* shared with the dealer, indicates the number of
     * anchors currently not connected
     */
    std::shared_ptr<std::atomic_int> dead_anchors = nullptr;

    /* thread controlling variables */
    std::atomic_bool stop_working = false;

    /* reference to the actual receiver thread */
    std::thread receiver_thread;

    /* Shared fifo queue between the receiver and the workers;
     * concurrency is managed within the queue object
     *   Note that this is also the control variable to check
     * if the receiver is running, so when the receiver is not
     * running this shall be nullptr.
     */
    std::shared_ptr<sync_queue> raw_packets_queue = nullptr;

    /* Shared deserialized-packets container;
     * This container is shared only among workers, the receiver
     * may only create it and pass it around
     */
    std::shared_ptr<collector> packet_collector = nullptr;

    /* threads that deserialize raw buffers into packets */
    std::vector<std::shared_ptr<worker>> workers;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;
};

#endif // !RECEIVER_H_INCLUDED
