#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED
#pragma once

#include "point2d.h"

/* 
 * A Sample is an RSSI measurement and its 
 * reference anchor position
 */
class sample
{
public:
    sample(
        point2d anchor_position_in = point2d(), int rssi_in = 0) :
        anchor_position(anchor_position_in), rssi(rssi_in) {}

    point2d anchor_position;
    int     rssi;
};

#endif // !SAMPLE_H_INCLUDED
