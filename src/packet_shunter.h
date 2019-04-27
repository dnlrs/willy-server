#ifndef PACKET_SHUNTER_H_INCLUDED
#define PACKET_SHUNTER_H_INCLUDED
#pragma once

#include "db.h"
#include "cfg.h"
#include "packet.h"
#include "device.h"
#include <string>
#include <cstdint>
#include <map>
#include <mutex>

constexpr long int default_max_container_size = 1000;
constexpr uint64_t default_max_timestamp_diff = 10; // seconds

class packet_shunter {

public:
    /* note: also opens a satabase connection */
    packet_shunter(
        std::shared_ptr<cfg::configuration> context_in, 
        int anchors_nr);
    ~packet_shunter();

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

private:
    /* Sets up and calls the localization routine
     *  
     * Retrieves anchors position from system configuration and
     * calls the localization routine.
     * */
    device process_readings(
        packet new_packet, 
        std::map<uint64_t, int32_t> readings);

    /* Stores packet and device data into persistent storage */
    void store_data(
        packet new_packet, 
        device new_device);

    /* Cleans the container from old packets and readings
     * 
     * Retrieves current time and deletes from container all references
     * to packets older than $(default_max_timestamp_diff) seconds.
     * */
    void clean_container();


    int anchors_number = 0;

    /* [packet hash -> [anchor mac -> rssi measurement] */
    std::map<std::string, std::map<uint64_t, int32_t>> rssi_readings;
    /* [packet hash -> timestamp] */
    std::map<std::string, uint64_t> timestamps;

    std::mutex packets_lock;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;

    /* database connection */
    db::database db_storage;
};

#endif // !PACKET_SHUNTER_H_INCLUDED