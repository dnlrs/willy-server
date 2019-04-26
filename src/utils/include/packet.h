#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED
#pragma once

#include "utils.h"
#include <cstdint> /* for uint64_t */
#include <sstream>
#include <winsock2.h>

#define MAC_LENGTH 6
#define MD5_HASH_LENGTH 32
#define MAX_SSID_LENGTH 32

class packet {

public:

    packet(
        uint32_t in_channel, 
        int32_t  in_rssi,
        uint32_t in_sequence_ctrl, 
        uint32_t in_ssid_length,
        uint64_t in_timestamp, 
        uint64_t in_device_mac,
        uint64_t in_anchor_mac, 
        std::string in_ssid,
        std::string in_hash) :
            channel(in_channel),
            rssi(in_rssi),
            sequence_ctrl(in_sequence_ctrl),
            ssid_length(in_ssid_length),
            timestamp(in_timestamp),
            device_mac(in_device_mac),
            anchor_mac(in_anchor_mac),
            ssid(in_ssid),
            hash(in_hash) {}

    std::string to_string() 
    {
        return std::string(
            "device mac: "     + mac_int2str(device_mac) +
            " rssi: "          + std::to_string(rssi) +
            (anchor_mac != 0 ? ("anchor mac: " + mac_int2str(anchor_mac)) : "") +
            (ssid_length > 0 ? (" ssid: " + ssid) : "ssid: none") +
            " channel: "       + std::to_string(channel) +
            " sequence_ctrl: " + std::to_string(sequence_ctrl) +
            " hash: " + hash);
    }

    uint32_t channel;
    int32_t  rssi;
    uint32_t sequence_ctrl;
    uint32_t ssid_length;
    uint64_t timestamp;
    
    uint64_t device_mac;
    uint64_t anchor_mac = 0; // placeholder

    std::string ssid;
    std::string hash;
};

#endif // !PACKET_H_INCLUDED

