#include "packet_shunter.h"
#include "shunter_exception.h"
#include "localization.h"
#include <cassert>

packet_shunter::packet_shunter(int anchors_nr) :
    anchors_number(anchors_nr),
    db_storage("database.db", anchors_nr)
{   
    try {
        db_storage.open(true);
    }
    catch (db::db_exception& dbe) {
        throw shunter_exception("ctor failed because database failed");
    }
}

packet_shunter::~packet_shunter()
{
    db_storage.quit();
}

void
packet_shunter::submit_packet(packet new_packet)
{
    std::unique_lock<std::mutex> guard(packets_lock);

    std::string hash       = new_packet.hash;
    int32_t     rssi       = new_packet.rssi;
    uint64_t    anchor_mac = new_packet.anchor_mac;
    
    rssi_readings[hash][anchor_mac] = rssi;
    timestamps[hash] = new_packet.timestamp;

    if (rssi_readings[hash].size() == anchors_number) {

        timestamps.erase(hash);
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
            context->get_anchor_position(
                        reading.first,      /* anchor mac */
                        anchor_coordinates  /* returned pair */);
        
        if (valid_position == false)
            throw shunter_exception("no anchor exists with specified mac");
        
        Point2d anchor_point(
                    anchor_coordinates.first, 
                    anchor_coordinates.second);

        measurements.push_back(std::make_pair(anchor_point, reading.second));
    }

    assert(measurements.size() == anchors_number);

    Point2d device_position = weighted_loc(measurements);

    device rval(
        new_packet.device_mac, new_packet.timestamp, 
        device_position.m_x(), device_position.m_y());

    debuglog("new device localized:\n" + rval.to_string());
    return rval;
}

void
packet_shunter::store_data(
    packet new_packet,
    device new_device)
{
    try {
        db_storage.add_packet(new_packet, new_packet.anchor_mac);
        db_storage.add_device(new_device);
    }
    catch (db::db_exception& dbe) {
        throw shunter_exception("store_data: failed, persistence layer fail");
    }
}

void
packet_shunter::clean_container()
{
    uint64_t current_time = get_current_time();

    for (auto reading : rssi_readings) {

        std::string hash      = reading.first;
        uint64_t    hash_time = timestamps[reading.first];

        if (hash_time - current_time > default_max_timestamp_diff) {
            timestamps.erase(hash);
            rssi_readings.erase(hash);
        }
    }
}