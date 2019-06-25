#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED
#pragma once

#include "ip_addr.h"
#include "mac_addr.h"
#include "fingerprint.h"
#include "utils.h"

#include <cstdint> /* for uint64_t */
#include <sstream>
#include <winsock2.h>

class packet
{
public:

    packet(
        uint32_t in_channel       = 0,
        int32_t  in_rssi          = 0,
        uint32_t in_sequence_ctrl = 0,
        uint32_t in_ssid_length   = 0,
        uint64_t in_timestamp     = 0,
        mac_addr in_device_mac    = mac_addr(),
        mac_addr in_anchor_mac    = mac_addr(),
        std::string in_ssid = "",
        std::string in_hash = "",
        fingerprint in_fp = fingerprint()) :
            channel(in_channel),
            rssi(in_rssi),
            sequence_ctrl(in_sequence_ctrl),
            ssid_length(in_ssid_length),
            timestamp(in_timestamp),
            device_mac(in_device_mac),
            anchor_mac(in_anchor_mac),
            ssid(in_ssid),
            hash(in_hash),
            fp(in_fp) {}

    bool operator<(const packet& other) const { return hash < other.hash; }

    std::string str()
    {
        return std::string(
            "\n\tdevice mac: " + device_mac.str() +
            "\n\trssi: "      + std::to_string(rssi) +
            "\n\ttimestamp: " + std::to_string(timestamp) +
            (anchor_mac.is_valid() ? ("\n\tanchor mac: " + anchor_mac.str()) : "") +
            ((ssid_length > 0)     ? ("\n\tssid: " + ssid) : "") +
            "\n\tchannel: "       + std::to_string(channel) +
            "\n\tsequence_ctrl: " + std::to_string(sequence_ctrl) +
            "\n\thash: "          + hash +
            "\n\tfingerprint: "  + fp.str());
    }

public:

    static const int md5_hash_length = 32;
    static const int max_ssid_length = 32;

    uint32_t channel;
    int32_t  rssi;
    uint32_t sequence_ctrl;
    uint32_t ssid_length;
    uint64_t timestamp;

    mac_addr device_mac;
    mac_addr anchor_mac;

    std::string ssid;
    std::string hash;

    fingerprint fp;
};

#endif // !PACKET_H_INCLUDED
