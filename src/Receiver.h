#ifndef RECEIVER_H_INCLUDED
#define RECEIVER_H_INCLUDED
#pragma once

#include "Dealer.h"
#include "sync_queue.h"
#include <thread>
#include <atomic>

constexpr long int default_sleep_ms    = 10;
constexpr long int select_timeout_sec  = 0;
constexpr long int select_timeout_usec = 20000;

class Receiver
{

public:
    Receiver(Dealer& dealer_ref, int disconnected_anchors);
	~Receiver();

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
    

    // reference to dealer
    Dealer& broker;

    // thread controlling variables
    std::atomic_int  disconnected_anchors = 0;
    std::atomic_bool stop_working         = false;

    // reference to the actual receiver thread
    std::thread receiver_thread;

    /* shared fifo queue between the receiver and the workers;
     * concurrency is managed within the queue object */
    std::shared_ptr<sync_queue> raw_packets_queue = nullptr;
};

#endif