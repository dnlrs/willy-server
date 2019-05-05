#include "collector.h"
#include "coll_exception.h"
#include <cassert>

collector::collector(
    std::shared_ptr<cfg::configuration> context_in) :
    context(context_in)
{
    anchors_number = context->get_anchors_number();
    try {
        db_storage = db::database("database.db", anchors_number);
        db_storage.open(true);
    }
    catch (db::db_exception& dbe) {
        throw coll_exception("colletor ctor: "
            "failed because database failed\n" + std::string(dbe.what()));
    }
}

collector::~collector()
{
    db_storage.quit();
}

void
collector::submit_packet(packet new_packet)
{
    std::unique_lock<std::mutex> guard(packets_lock);

    std::string hash       = new_packet.hash;
    int32_t     rssi       = new_packet.rssi;
    mac_addr    anchor_mac = new_packet.anchor_mac;

    rssi_readings[hash][anchor_mac] = rssi;
    timestamps[hash] = new_packet.timestamp;

    if (rssi_readings[hash].size() == anchors_number) {
        timestamps.erase(hash);
        auto readings = rssi_readings.extract(hash);

        guard.unlock();
        device new_device = process_readings(new_packet, readings.mapped());
        store_data(new_packet, new_device);
        guard.lock();
    }

    if (timestamps.size() > default_max_container_size)
        clean_container();
}

device
collector::process_readings(
    packet new_packet,
    std::map<mac_addr, int32_t> readings)
{
    std::vector<sample> measurements;

    for (auto reading : readings) {
        std::pair<double, double> anchor_coordinates;
        bool valid_position =
            context->get_anchor_position(
                reading.first,      /* anchor mac */
                anchor_coordinates  /* returned pair */);

        if (valid_position == false)
            throw coll_exception("collector::process_readings: "
                "no anchor exists with specified mac");

        point2d anchor_position(anchor_coordinates);
        measurements.push_back(
            sample(anchor_position, reading.second));
    }

    assert(measurements.size() == anchors_number);
    point2d device_position = locator.localize_device(measurements);

#ifdef _DEBUG

    device rval(
        new_packet.device_mac, new_packet.timestamp,
        device_position.x, device_position.y);

    debuglog("localized:", rval.str());
    return rval;

#else

    return device(
        new_packet.device_mac, new_packet.timestamp,
        device_position.x, device_position.y);
#endif
}

void
collector::store_data(
    packet new_packet,
    device new_device)
{
    try {
        db_storage.add_packet(new_packet, new_packet.anchor_mac);
        db_storage.add_device(new_device);
    }
    catch (db::db_exception& dbe) {
        if (dbe.why() == db::db_exception::type::error)
            throw coll_exception("collector::store_data: "
                "failed, persistence layer fail\n" + std::string(dbe.what()));
    }
}

void
collector::clean_container()
{
    debuglog("collector::clean_container: cleaning...");
    uint64_t current_time = get_current_time();

    std::vector<std::string> old_hashes;
    for (auto timestamp : timestamps) {
        if ((current_time - timestamp.second) >= default_max_timestamp_diff) {
            old_hashes.push_back(timestamp.first);
        }
    }

    for (auto hash : old_hashes) {
        timestamps.erase(hash);
        rssi_readings.erase(hash);
    }
}
