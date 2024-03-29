#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED
#pragma once

#include "cfg.h"
#include "collector.h"
#include "sized_buffer.h"
#include "sync_queue.h"
#include <atomic>
#include <thread>

class worker
{
public:
    worker(
        std::shared_ptr<cfg::configuration> context_in,
        std::shared_ptr<sync_queue> shared_raw_buffers,
        std::shared_ptr<collector>  collector_in);
    ~worker();

    /* Deploys the worker thread */
    void start();

    /* Stops the worker thread */
    void stop();

    /* joins the worker thread */
    void finish();

private:
    /* Strategy:
     *  1. get a raw message from the queue shared with the receiver
     *  2. deserialize and create a new packet
     *  3. submit packet to the collector
     *    (within the collector)
     *    3.a if packet has been received from all anchors perform
     *        localization
     *    3.b insert new device position into database
     *    3.c if necessary clean the container
     */
    void service();

    /* Calls stop() then finish() (blocking) */
    void shutdown_worker();

    /* Deserializes a raw data buffer into a packet */
    packet deserialize(sized_buffer buffer);

private:
    /* thread controlling variables */
    std::atomic_bool stop_working = false;

    /* Shared fifo queue between the receiver and the workers;
     * concurrency is managed within the queue object
     */
    std::shared_ptr<sync_queue> raw_packets_queue = nullptr;

    /* Shared deserialized packets container;
     * This container is shared with other worker threads.
     * Within this collector there is the database connection
     * and necessary calls for device localization
     */
    std::shared_ptr<collector> packet_collector = nullptr;

    /* reference to the actual worker thread */
    std::thread worker_thread;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;
};

#endif // !WORKER_H_INCLUDED