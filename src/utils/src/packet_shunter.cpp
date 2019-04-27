#include "packet_shunter.h"

packet_shunter::packet_shunter(int in_anchors_number) :
    anchors_number(in_anchors_number)
{

}

void
packet_shunter::submit_packet(packet new_packet)
{
    std::unique_lock<std::mutex> guard(packets_lock);

    std::string hash    = new_packet.hash;
    int32_t     rssi    = new_packet.rssi;
    uint64_t anchor_mac = new_packet.anchor_mac;
    
    
    auto find_hash = rssi_readings.find(hash);

    if (find_hash == rssi_readings.end())
        packets[hash] = new_packet;

    rssi_readings[hash][anchor_mac] = rssi;

    if (rssi_readings[hash].size() == anchors_number) {
        // TODO: extrack from container
        // unlock mutex
        // localize
        // insert in db
        // return
    }
}