#ifndef PACKET_SHUNTER_H_INCLUDED
#define PACKET_SHUNTER_H_INCLUDED
#pragma once

#include "packet.h"
#include <string>
#include <cstdint>
#include <map>
#include <mutex>


class packet_shunter {

public:
    packet_shunter(int in_anchors_number);
    ~packet_shunter();

    void submit_packet(packet new_packet);

private:

    int anchors_number = 0;

    /* [packet hash -> packet] */
    std::map<std::string, packet> packets;
    /* [packet hash -> [anchor mac -> rssi measurement] */
    std::map<std::string, std::map<uint64_t, int32_t>> rssi_readings;

    std::mutex packets_lock;


};

#endif // !PACKET_SHUNTER_H_INCLUDED