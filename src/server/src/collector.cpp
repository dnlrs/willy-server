#include "collector.h"
#include "coll_exception.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <utility>

collector::collector(
    std::shared_ptr<cfg::configuration> context_in) :
    context(context_in)
{
    anchors_number = context->get_anchors_number();
    last_flushed   = 0;
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

    int32_t  rssi       = new_packet.rssi;
    mac_addr anchor_mac = new_packet.anchor_mac;

    packets[new_packet][anchor_mac] = rssi;

    if (packets[new_packet].size() == anchors_number) {
        auto readings = packets.extract(new_packet);

        guard.unlock();
        device new_device = process_readings(new_packet, readings.mapped());
        store_data(new_packet, new_device);
        guard.lock();
    }

    if (packets.size() > collector_max_container_size)
        clean_container();
}

void
collector::flush()
/*
 * Store into database all device localizations older than
 * current time (minus overlap).
 * Remove the device entry if after removing all positions
 * associated with old timestamps, there are no timestamps
 * left (nor positions).
 */
{
    std::unique_lock<std::mutex> guard(devices_lock);
    
    /* check if the right time has passed since last time */
    uint64_t now = get_current_time() - collector_flushing_overlap;
    if (now - last_flushed < collector_flushing_interval)
        return;
    
    auto dev = devices.begin();
    while (dev != devices.end()) {
        auto pos = dev->second.begin();
        while (pos != dev->second.end()) {
            uint64_t timestamp = pos->first;
            if (timestamp < now) {
                device new_device(
                    dev->first.mac,          /* device mac */
                    timestamp,               /* timestamp  */
                    pos->second.first.x,     /* x position */
                    pos->second.first.y);    /* y position */

                debuglog("[device]", new_device.str());

                try {
                    db_storage.add_device(new_device);
                }
                catch (db::db_exception& dbe) {
                    if (dbe.why() == db::db_exception::type::error)
                        throw coll_exception("collector::store_data: "
                            "failed, persistence layer fail\n" + std::string(dbe.what()));
                }

                /* remove this centroid and update iterator */
                pos = dev->second.erase(pos);
            }
            else {
                pos++;
            }
        }

        if (dev->second.empty())
            dev = devices.erase(dev);
        else
            dev++;
    }

    /* update last flushed */
    last_flushed = now;
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
        measurements.push_back(sample(anchor_position, reading.second));
    }

    assert(measurements.size() == anchors_number);
    point2d device_position = locator.localize_device(measurements);

    return device(
        new_packet.device_mac, 
        new_packet.timestamp,
        device_position.x, 
        device_position.y);
}

void
collector::store_data(
    packet new_packet,
    device new_device)
/*
 * Insert packet immediately into database. Defer device
 * insertion because its position, given a timestamp may
 * be an average among multiple estimated positions.
 */
{
    try {
        db_storage.add_packet(new_packet, new_packet.anchor_mac);
    }
    catch (db::db_exception& dbe) {
        if (dbe.why() == db::db_exception::type::error)
            throw coll_exception("collector::store_data: "
                "failed, persistence layer fail\n" + std::string(dbe.what()));
    }

    std::unique_lock<std::mutex> guard(devices_lock);

    auto old_centroid = devices[new_device][new_device.timestamp];
    devices[new_device][new_device.timestamp] =
        locator.update_centroid(old_centroid, new_device.position);
}

void
collector::clean_container()
{
    uint64_t now = get_current_time();

    auto pack = packets.begin();
    while (pack != packets.end()) {
        if ((now - pack->first.timestamp) >= collector_max_timestamp_diff)
            pack = packets.erase(pack);
        else
            pack++;
    }
}
