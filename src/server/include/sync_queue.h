#ifndef ATOMIC_BLOCKING_QUEUE_H_INCLUDED
#define ATOMIC_BLOCKING_QUEUE_H_INCLUDED
#pragma once

#include "logger.h"
#include "sized_buffer.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

/* Maximum waiting time for a thread on this queue */
constexpr long int sync_queue_waiting_time_ms = 200;

/* FIFO blocking queue with concurrency management
 *
 * When a thread does pop() on this queue, if an element is
 * available then it's immediately returned. If no elements
 * are available then the thread blocks on a condition
 * variable waiting for a producer to push() a new element.
 *   After a certain amount of time, if no new elements are
 * pushed then the thread is returned an empty element.
 */
class sync_queue
{
public:

    sync_queue() {}
    ~sync_queue() {}

    void push(sized_buffer in) {
        std::unique_lock<std::mutex> guard(fifo_queue_lock);
        fifo_queue.push(in);

        if (consumers_nr > 0)
            consumer.notify_all();
    }

    sized_buffer pop() {
        std::unique_lock<std::mutex> guard(fifo_queue_lock);
        sized_buffer rval;

        if (fifo_queue.size() > 0) {
            rval = fifo_queue.front();
            fifo_queue.pop();
        }
        else {
            consumers_nr++;

            if (consumer.wait_for(guard, 
                    std::chrono::milliseconds(sync_queue_waiting_time_ms),
                    [this]() -> bool { return fifo_queue.size() > 0; })) {
                rval = fifo_queue.front();
                fifo_queue.pop();
            }
            else {
                rval.msg_size = 0;
                rval.msg.clear();
            }

            consumers_nr--;
        }

        return rval;
    }

private:
    std::queue<sized_buffer> fifo_queue;
    std::mutex fifo_queue_lock;

    /* Consumers are threads waiting for the queue to return ad object */
    std::atomic_int         consumers_nr = 0;
    std::condition_variable consumer;
};

#endif // !ATOMIC_BLOCKING_QUEUE_H_INCLUDED
