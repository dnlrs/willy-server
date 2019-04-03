#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED
#pragma once


class device {

public:
    device() : mac(0), timestamp(0), pos_x(0), pos_y(0) {}

    device(
        uint64_t mac_in, uint64_t timestamp_in, 
        double pos_x_in, double pos_y_in) :
            mac(mac_in), timestamp(timestamp_in),
            pos_x(pos_x_in), pos_y(pos_y_in) {}

public:
    uint64_t mac;
    uint64_t timestamp;
    double pos_x;
    double pos_y;
};

#endif // !DEVICE_H_INCLUDED
