#include "logger.h"
#include "mac_addr.h"
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
    shutdown_worker();
}

void
worker::start()
{
    // this assures a clean start
    shutdown_worker();

    stop_working  = false;
    worker_thread = std::thread(&worker::service, this);
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
worker::shutdown_worker()
{
    stop();
    finish();
}

void
worker::service()
{
    debuglog("worker thread:", std::this_thread::get_id());
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
    uint32_t* pmsg = (uint32_t*)buffer.msg.data();

    uint32_t channel       = ntohl(pmsg[0]); // channel
    int32_t  rssi          = ntohl(pmsg[1]); // rssi
    uint32_t sequence_ctrl = ntohl(pmsg[2]); // sequence control
    uint64_t timestamp     = ntohl(pmsg[3]); // timestamp
    int32_t  ssid_length   = ntohl(pmsg[4]); // ssid length

    uint8_t* pmsg_byte = (uint8_t*)&pmsg[5];

    // device mac
    mac_addr device_mac;
    for (int i = 0; i < mac_addr::mac_length; i++) {
        device_mac[i] = *pmsg_byte;
        pmsg_byte++;
    }

    // packet hash
    char raw_hash[packet::md5_hash_length + 1];
    for (int i = 0; i < packet::md5_hash_length; i++) {
        raw_hash[i] = *pmsg_byte;
        pmsg_byte++;
    }
    raw_hash[packet::md5_hash_length] = '\0';
    std::string hash(raw_hash);

    // ssid
    char raw_ssid[packet::max_ssid_length + 1];
    for (int32_t i = 0; i < ssid_length; i++) {
        raw_ssid[i] = *pmsg_byte;
        pmsg_byte++;
    }
    raw_ssid[ssid_length] = '\0';
    std::string ssid(raw_ssid);

#ifdef _DEBUG

    packet rval(channel, rssi, sequence_ctrl,
        ssid_length, timestamp,
        device_mac, mac_addr(), ssid, hash);

    debuglog("deserialized: ", rval.str());
    return rval;

#else

    return packet(channel, rssi, sequence_ctrl,
        ssid_length, timestamp,
        device_mac, mac_addr(), ssid, hash);

#endif
}