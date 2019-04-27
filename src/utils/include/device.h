#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED
#pragma once

#include "utils.h"
#include <cstdint>
#include <string>
class device {

public:
    device() : mac(0), timestamp(0), pos_x(0), pos_y(0) {}

    device(
        uint64_t mac_in, uint64_t timestamp_in, 
        double pos_x_in, double pos_y_in) :
            mac(mac_in), timestamp(timestamp_in),
            pos_x(pos_x_in), pos_y(pos_y_in) {}

    std::string to_string()
    {
        return std::string(
            "mac: "       + mac_int2str(mac) +
            " timstamp: " + std::to_string(timestamp) +
            " X: "        + std::to_string(pos_x) +
            " Y: "        + std::to_string(pos_y));
    }

public:
    uint64_t mac;
    uint64_t timestamp;
    double pos_x;
    double pos_y;
};

#endif // !DEVICE_H_INCLUDED
