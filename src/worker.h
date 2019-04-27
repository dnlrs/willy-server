#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED
#pragma once

#include "atomic_blocking_queue.h"
#include "packet.h"
#include "sized_buffer.hpp"
#include <atomic>
#include <thread>

class worker {
	
public:
	worker(std::shared_ptr<sync_queue> shared_raw_buffers);
	~worker();
	
	void start();
	void stop();
	void finish();
	
private:
    /* Strategy:
     *  1. get a raw message from the queue shared with the receiver
     *  2. deserialize and create a new packet
     *  3. insert the new packet in the container
     *    3.a if packet has been received from all anchors perform 
     *        localization
     *    3.b insert new device position into database
     *    3.c if necessary clean the container
     */
    void service();

    /* Deserializes a raw data buffer into a packet */
    packet deserialize(sized_buffer buffer);


    std::shared_ptr<sync_queue> raw_packets_queue = nullptr;

    std::atomic_bool stop_working = false;

    std::thread worker_thread;
};

#endif // !WORKER_H_INCLUDED