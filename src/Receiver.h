#ifndef RECEIVER_H_INCLUDED
#define RECEIVER_H_INCLUDED
#pragma once

#include "cfg.h"
#include "collector.h"
#include "sync_queue.h"
#include "worker.h"
#include <thread>
#include <atomic>

constexpr long int default_sleep_ms    = 10;
constexpr long int select_timeout_sec  = 0;
constexpr long int select_timeout_usec = 20000;

constexpr int default_workers_number = 4;

// forward declaration
class dealer;

class receiver
{

public:
    receiver(
        dealer& dealer_ref, 
        std::shared_ptr<cfg::configuration> context_in,
        int anchors_number);
	~receiver();

    void start();
    void stop();
    void finish();

    void notify_anchor_connected();

private:

    /* Strategy:
     *   While all anchors are connected read data from the sockets
     * and push buffers into the chared queue for deserialization.
     * If an anchor disconnects, continue to read data from sockets
     * (in order to empty socket buffers) but ignore it.
     */
    void service();

    int get_workers_number();
    

private:
    // reference to dealer
    dealer& broker;

    int anchors_nr = 0;

    /* thread controlling variables */
    std::atomic_int  disconnected_anchors = 0;
    std::atomic_bool stop_working         = false;

    /* reference to the actual receiver thread */
    std::thread receiver_thread;

    /* shared fifo queue between the receiver and the workers;
     * concurrency is managed within the queue object 
     * */
    std::shared_ptr<sync_queue> raw_packets_queue = nullptr;

    /* shared deserialized-packets container;
     * This container is shared only among workers, the receiver 
     * may only create it and pass it around 
     * */
    std::shared_ptr<collector> packet_collector = nullptr;

    /* threads that deserialize raw buffers into packets */
    std::vector<worker> workers;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;
};

#endif