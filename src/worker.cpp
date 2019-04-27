#include "logger.h"
#include "worker.h"

worker::worker(
    std::shared_ptr<cfg::configuration> context_in,
    std::shared_ptr<sync_queue> shared_raw_buffers,
    std::shared_ptr<collector>  collector_in) :
        context(context_in),
        raw_packets_queue(shared_raw_buffers),
        packet_collector(collector_in) {}

worker::~worker()
{
    stop();
    finish();
}

void
worker::start()
{
    /* Assuring a clean state before starting */
    stop();
    finish();

    stop_working  = false;
    worker_thread = std::thread(&service, this);
}

void
worker::stop()
{
    stop_working = true;
}

void
worker::finish()
{
    if (worker_thread.joinable())
        worker_thread.join();
}

void
worker::service()
{
    while (stop_working == false) {
        sized_buffer buffer = raw_packets_queue->pop();

        if (buffer.msg_size == 0)
            continue; // empty buffer means empty queue

        packet new_packet     = deserialize(buffer);
        new_packet.anchor_mac = buffer.anchor_mac;

        packet_collector->submit_packet(new_packet);
    }
}



packet
worker::deserialize(sized_buffer buffer)
{
    uint32_t* pmsg = (uint32_t*) buffer.msg.data();

    uint32_t channel       = ntohl(*pmsg++); // channel
    int32_t  rssi          = ntohl(*pmsg++); // rssi
    uint32_t sequence_ctrl = ntohl(*pmsg++); // sequence control
    uint64_t timestamp     = ntohl(*pmsg++); // timestamp
    int32_t  ssid_length   = ntohl(*pmsg++); // ssid length

    uint8_t* pmsg_byte = (uint8_t*) pmsg;

    // device mac
    uint64_t device_mac = 0;
    for (int i = 0; i < MAC_LENGTH; i++) {
        ((uint8_t*) device_mac)[i] = *pmsg_byte;
        pmsg_byte++;
    }

    // packet hash
    std::string hash;
    for (int i = 0; i < MD5_HASH_LENGTH; i++) {
        hash[i] = *pmsg_byte;
        pmsg_byte++;
    }
    hash[MD5_HASH_LENGTH] = '\0';

    // ssid
    std::string ssid;
    for (uint32_t i = 0; i < ssid_length; i++) {
        ssid[i] = *pmsg_byte;
        pmsg_byte++;
    }
    ssid[ssid_length] = '\0';

    packet rval (channel, rssi, sequence_ctrl, 
        ssid_length, timestamp, 
        device_mac, 0, ssid, hash);

    debuglog(rval.to_string());

    return rval;
}