#include "logger.h"
#include "mac_addr.h"
#include "packet.h"
#include "worker.h"

using std::vector;
using std::string;
using std::shared_ptr;

worker::worker(
    shared_ptr<cfg::configuration> context_in,
    shared_ptr<sync_queue> shared_raw_buffers,
    shared_ptr<collector>  collector_in) :
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

        packet_collector->flush();

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
    /* uint32_t* mode */
    uint32_t* pmsg = (uint32_t*)buffer.msg.data();

    uint32_t channel       = ntohl(pmsg[0]); // channel
    int32_t  rssi          = ntohl(pmsg[1]); // rssi
    uint64_t timestamp     = ntohl(pmsg[2]); // timestamp
    uint32_t sequence_ctrl = ntohl(pmsg[3]); // sequence control
    uint32_t ssid_num      = ntohl(pmsg[4]); // number of SSIDs
    
    fingerprint pfp;

    pfp.tag_presence = ntohl(pmsg[5]); // tags presence

    /* uint8_t* mode */
    uint8_t* pmsg_byte = (uint8_t*) &pmsg[6];
    uint8_t* dst = nullptr;

    /* supported rates */
    dst = (uint8_t*)&pfp.supported_rates;
    for (int i = 0; i < 8; i++)
        *dst++ = *pmsg_byte++;
    
    /* ht capabilities */
    if (is_tag_set(TAG_HT_CAPABILITY, &pfp.tag_presence)) {
        dst = (uint8_t*) &(pfp.ht_capability_info);
        for (int i = 0; i < HT_CAPABILITIES_LEN; i++)
            *dst++ = *pmsg_byte++;
    }

    /* extended capabilities */
    if (is_tag_set(TAG_EXTENDED_CAPABILITIES, &pfp.tag_presence)) {
        dst = (uint8_t*) &(pfp.ext_extended_capabilities);
        for (int i = 0; i < EXT_CAPABILITIES_LEN; i++)
            *dst++ = *pmsg_byte++;
    }

    /* interworking */
    if (is_tag_set(TAG_INTERWORKING, &pfp.tag_presence)) {
        pfp.iw_interworking = *pmsg_byte++;
    }

    /* Multi Band */
    if (is_tag_set(TAG_MULTI_BAND, &pfp.tag_presence)) {
        pfp.multi_band_id      = *pmsg_byte++;
        pfp.multi_band_channel = *pmsg_byte++;
    }

    /* VHT Capabilities */
    if (is_tag_set(TAG_VHT_CAPABILITY, &pfp.tag_presence)) {
        dst = (uint8_t*) &(pfp.vht_capabilities_info);
        for (int i = 0; i < VHT_CAPABILITIES_LEN; i++)
            *dst++ = *pmsg_byte++;
    }

    /* device mac */
    mac_addr device_mac;
    for (int i = 0; i < mac_addr::mac_length; i++) {
        device_mac[i] = *pmsg_byte++;
    }

    /*
        SSIDs
            |len|"ssid-name"|
    */
    for (int i = 0; i < ssid_num; i++) {
        uint8_t ssid_len = *pmsg_byte++;
        
        string ssid((char*) pmsg_byte, ssid_len);
        pmsg_byte += ssid_len;

        pfp.ssid_list.push_back(ssid);
    }

    /* hash */
    string hash((char*) pmsg_byte, packet::md5_hash_length);
    pmsg_byte += packet::md5_hash_length;

    if (((uint8_t*)pmsg_byte - (uint8_t*)buffer.msg.data()) != buffer.msg_size )
        debuglog("[deserialize] incongruent buffer END!");
    

    /* for compatibility with old interface */
    std::string ssid;
    int ssid_length = 0;
    if (pfp.ssid_list.size() > 0) {
        ssid = pfp.ssid_list[0];
        ssid_length = ssid.size();
    }

#ifdef _DEBUG

    debuglog("----- FINGERPRINT ", pfp.str());

    packet rval(channel, rssi, sequence_ctrl,
        ssid_length, timestamp,
        device_mac, mac_addr(), ssid, hash, pfp);

    debuglog("[packet]", rval.str());
    return rval;

#else

    return packet(channel, rssi, sequence_ctrl,
        ssid_length, timestamp,
        device_mac, mac_addr(), ssid, hash);

#endif
}
