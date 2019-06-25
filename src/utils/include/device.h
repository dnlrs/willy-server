#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED
#pragma once

#include "utils.h"
#include "mac_addr.h"
#include "point2d.h"
#include "fingerprint.h"

#include <cstdint>
#include <string>

class device {

public:
    device(
        mac_addr mac = mac_addr(), 
        uint64_t timestamp = 0, 
        double pos_x = 0, 
        double pos_y = 0) :
            mac(mac), timestamp(timestamp),
            position(pos_x, pos_y) {}

    std::string str()
    {
        return std::string(
            "\n\tmac: "        + mac.str() +
            "\n\ttimestamp: " + std::to_string(timestamp) +
            "\n\tposition: "  + position.str() +
            "\n\tfingerprint " +fp.str());
    }

    bool operator<(const device& other) const { return mac < other.mac; }

public:
    mac_addr mac;
    uint64_t timestamp;

    point2d position;

    fingerprint fp;
};

#endif // !DEVICE_H_INCLUDED
