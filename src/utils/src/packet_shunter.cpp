#include "packet_shunter.h"
#include "localization.h"
#include <cassert>

packet_shunter::packet_shunter(int anchors_nr) :
    anchors_number(anchors_nr)
{

}

void
packet_shunter::submit_packet(packet new_packet)
{
    std::unique_lock<std::mutex> guard(packets_lock);

    std::string hash    = new_packet.hash;
    int32_t     rssi    = new_packet.rssi;
    uint64_t anchor_mac = new_packet.anchor_mac;
    
    
    //auto find_hash = rssi_readings.find(hash);

    //if (find_hash == rssi_readings.end())
    //    packets[hash] = new_packet;

    rssi_readings[hash][anchor_mac] = rssi;

    if (rssi_readings[hash].size() == anchors_number) {
        // TODO: extrack from container
        // unlock mutex
        // localize
        // insert in db
        // return
        
        auto readings = rssi_readings.extract(hash);
        
        guard.release();
        device new_device = process_readings(new_packet, readings.mapped());
        store_data(new_packet, new_device);
    }

    if (guard.owns_lock() == false)
        guard.lock();

    if (rssi_readings.size() > default_max_container_size)
        clean_container();
}

device
packet_shunter::process_readings(
    packet new_packet,
    std::map<uint64_t, int32_t> readings)
{
    std::vector<std::pair<Point2d, int>> measurements;

    for (auto reading : readings) {
        std::pair<double, double> anchor_coordinates;
        bool valid_position = 
            context->get_anchor_position(reading.first, anchor_coordinates);
        
        //if (valid_position == false)
        // TODO: check errors
        
        Point2d anchor_point(anchor_coordinates.first, anchor_coordinates.second);

        measurements.push_back(std::make_pair(anchor_point, reading.second));
    }

    assert(measurements.size() == anchors_number);

    Point2d device_position = weighted_loc(measurements);

    device rval(
        new_packet.device_mac, new_packet.timestamp, 
        device_position.m_x(), device_position.m_y());

    return rval;
}

void
packet_shunter::store_data(
    packet new_packet,
    device new_device)
{
}

void
packet_shunter::clean_container()
{

}