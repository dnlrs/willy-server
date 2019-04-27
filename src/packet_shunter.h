#ifndef PACKET_SHUNTER_H_INCLUDED
#define PACKET_SHUNTER_H_INCLUDED
#pragma once

#include "cfg.h"
#include "packet.h"
#include "device.h"
#include <string>
#include <cstdint>
#include <map>
#include <mutex>

constexpr long int default_max_container_size = 1000;

class packet_shunter {

public:
    packet_shunter(int anchors_nr);
    ~packet_shunter();

    void submit_packet(packet new_packet);

private:
    device process_readings(
        packet new_packet, 
        std::map<uint64_t, int32_t> readings);

    void store_data(
        packet new_packet, 
        device new_device);


    void clean_container();


    int anchors_number = 0;

    /* [packet hash -> packet] */
//    std::map<std::string, packet> packets;
    /* [packet hash -> [anchor mac -> rssi measurement] */
    std::map<std::string, std::map<uint64_t, int32_t>> rssi_readings;

    std::mutex packets_lock;

    std::shared_ptr<cfg::configuration> context;
};

#endif // !PACKET_SHUNTER_H_INCLUDED