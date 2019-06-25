#ifndef IPS_H_INCLUDED
#define IPS_H_INCLUDED
#pragma once

#include "circumference.h"
#include "ips_exception.h"
#include "point2d.h"
#include "sample.h"
#include <cmath>
#include <vector>

typedef struct s_boundaries
{
    double left;
    double upper;
    double right;
    double lower;
} boundaries;

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
        
        /* check if point is within boundaries */
        boundaries limits = create_boundaries(measurements);
        point2d rval = multilateration(measurements);

        if (is_within_boundaries(rval, limits) == false)
            throw ips_exception("ips::localize_device: out of boundaries device");

        return rval;
    }

    /* Returns the centroid and the number of points used to calculate it
     * 
     * ...given a centroid and the number of points userd to calculate 
     * it and a new point.
     */
    std::pair<point2d, int> update_centroid(
        std::pair<point2d, int> avg_pos,
        point2d new_pos) const
    {
        double sum_x = avg_pos.first.x * (double)avg_pos.second + new_pos.x;
        double sum_y = avg_pos.first.y * (double)avg_pos.second + new_pos.y;
        int k = avg_pos.second + 1;

        return std::make_pair(point2d(sum_x / k, sum_y / k), k);
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

    boundaries create_boundaries(
        const std::vector<sample> measurements) const;
    
    bool is_within_boundaries(point2d point, boundaries limits) const;
private:
    double A = -55;
    double n = 2.0; // reference
};

#endif // !IPS_H_INCLUDED
