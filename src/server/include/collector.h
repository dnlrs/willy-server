#ifndef COLLECTOR_H_INCLUDED
#define COLLECTOR_H_INCLUDED
#pragma once

#include "cfg.h"
#include "db.h"
#include "device.h"
#include "ips.h"
#include "packet.h"
#include <cstdint>
#include <string>
#include <map>
#include <mutex>

constexpr long int collector_max_container_size = 1000;
constexpr uint64_t collector_max_timestamp_diff = 10; // seconds

constexpr uint64_t collector_flushing_interval = 5; // seconds
constexpr uint64_t collector_flushing_overlap  = 1; // seconds

class collector
{
public:
    /* note: also opens a satabase connection */
    collector(
        std::shared_ptr<cfg::configuration> context_in);

    ~collector();

    /* Adds a new packet to the container
     *
     * If the packet has been received by all anchors then performs
     * localization and inserts the packet and the localized device
     * into the persistent storage (db).
     *
     * If the container reached the max allowed size then it is
     * cleaned. during the cleaning process other threads cannot
     * access the containter.
     *
     * This function will be executed by different worker threads.
     * */
    void submit_packet(packet new_packet);


    /* Synchronizes data in this container with the underneath database */
    void flush();

private:
    /* Sets up and calls the localization routine
     *
     * Retrieves anchors position from system configuration and
     * calls the localization routine.
     * */
    device process_readings(
        packet new_packet,
        std::map<mac_addr, int32_t> readings);

    /* Stores packet and device data into persistent storage */
    void store_data(
        packet new_packet,
        device new_device);

    /* Cleans the container from old packets and readings
     *
     * Retrieves current time and deletes from container all references
     * to packets older than $(collector_max_timestamp_diff) seconds.
     * */
    void clean_container();

private:
    int anchors_number = 0;

    /* localization system */
    ips locator;

    std::mutex packets_lock;
    /* [packet -> [anchor mac -> rssi measurement]] */
    std::map<packet, std::map<mac_addr, int32_t>> packets;

    std::mutex devices_lock;
    /* [device -> [timestamp -> [position_average, points_number]]] */
    std::map<device, std::map<uint64_t, std::pair<point2d, int>>> devices;

    /* a timestamp that indicates when the lash flush was done */
    uint64_t last_flushed = 0;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;

    /* database connection */
    db::database db_storage;
};

#endif // !COLLECTOR_H_INCLUDED
