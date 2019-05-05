#ifndef IPS_H_INCLUDED
#define IPS_H_INCLUDED
#pragma once

#include "circumference.h"
#include "ips_exception.h"
#include "point2d.h"
#include "sample.h"
#include <cmath>
#include <vector>

/* Indoor Positioning System - IPS */
class ips
{
public:
    ips() : A(-55), n(2.0) {}

    point2d localize_device(
        const std::vector<sample> measurements) const
    {
        size_t anchors_nr = measurements.size();

        if (anchors_nr == 0)
            throw ips_exception("ips::localize_device: no measurements provided");

        if (anchors_nr == 1)
            return unilateration(measurements[0]);

        return multilateration(measurements);
    }

private:
    /* simple log-normal shadowing path loss model */
    double get_distance_from_rssi(int rssi, double multiplier) const;

    /*
     * Returns the device position as if it were on the
     * right of the anchor, at measured distance.
     */
    point2d unilateration(const sample measurement) const;

    /* Returns the device position given any number (> 1) of samples */
    point2d multilateration(
        const std::vector<sample> measurements) const;

    /* Returns the intersection points of any number (> 1) of circumferences */
    bool get_all_intersection_points(
        const std::vector<circumference> circles,
        std::vector<point2d>& points_rval) const;

    /* Returns the centroid of a set of points */
    point2d get_centroid(const std::vector<point2d> points) const;

private:
    double A = -55;
    double n = 2.0; // reference
};

#endif // !IPS_H_INCLUDED
