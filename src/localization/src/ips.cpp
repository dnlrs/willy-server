#include "ips.h"

#include <cassert>
#include <limits>

double
ips::get_distance_from_rssi(int rssi, double multiplier) const
/*
 * LogNormal Shadowing Path Loss Model
 *
 *   RSSI(d) = - 10nlog(d) + A
 *
 * d: distance from anchor to device
 * n: path loss exponent
 * A: received signal strength in dBm at 1 metre
 * the log is 10-based
 *
 * The multiplier is a scaling factor used during
 * multilateration when there are circumferences
 * that do not intersecate.
 * 
 * ref: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3231493/
 */
{
    double exp = ((double)(rssi * multiplier) - A) / (-10 * n);
    return pow(10, exp);
}

point2d ips::unilateration(
    const sample measurement) const
{
    double distance   = get_distance_from_rssi(measurement.rssi, 1);

    return point2d(
        measurement.anchor_position.x + distance,
        measurement.anchor_position.y);
}

point2d ips::multilateration(
    const std::vector<sample> measurements) const
/*
 * ref: http://probr.ch/assets/files/probr-paper.pdf
 */
{
    std::vector<circumference> circumferences;
    std::vector<point2d>       intersections;
    double multiplier = 1;

    while (multiplier <= 10) {
        // clear data
        circumferences.clear();
        intersections.clear();

        // build circumferences
        for (auto measure : measurements) {
            circumferences.push_back(circumference(
                measure.anchor_position,
                get_distance_from_rssi(measure.rssi, multiplier)));
        }

        // calculate all intersections
        if (get_all_intersection_points(circumferences, intersections) == false) {
            multiplier += 0.1;
            continue;
        }
        else {
            // return intersection points centroid (device position)
            return get_centroid(intersections);
        }
    }

    throw ips_exception("ips::multilateration: "
        "could not intersect circumferences");
}

bool
ips::get_all_intersection_points(
    const std::vector<circumference> circles,
    std::vector<point2d>& rval) const
{
    for (auto c_i = circles.begin(); c_i < circles.end(); c_i++) {
        for (auto c_j = c_i + 1; c_j < circles.end(); c_j++) {
            std::vector<point2d> poi; // Points Of Intersection
            if (c_i->get_intersection_points(*c_j, poi) == false) {
                rval.clear();
                return false;
            }
            else {
                rval.insert(rval.end(), poi.begin(), poi.end());
            }
        }
    }

    return true;
}

point2d
ips::get_centroid(const std::vector<point2d> points) const
/*
 * C_x = (p1_x + p2_x + ... + pK_x) / K
 * C_y = (p1_y + p2_y + ... + pK_y) / K
 */
{
    double K = (double) points.size();

    double sum_x = 0;
    double sum_y = 0;
    for (auto point : points) {
        sum_x += point.x;
        sum_y += point.y;
    }

    return point2d((sum_x / K), (sum_y / K));
}

boundaries 
ips::create_boundaries(
    const std::vector<sample> measurements) const
{   
    boundaries rval = { 0 };
    rval.left  = std::numeric_limits<double>::max(); // min x
    rval.upper = std::numeric_limits<double>::min(); // max y
    rval.right = std::numeric_limits<double>::min(); // max x
    rval.lower = std::numeric_limits<double>::max(); // min y

    for (auto ms : measurements) {
        // left boundary
        if (ms.anchor_position.x < rval.left)
            rval.left = ms.anchor_position.x;

        // upper boundary
        if (ms.anchor_position.y > rval.upper)
            rval.upper = ms.anchor_position.y;

        // right boundary
        if (ms.anchor_position.x > rval.right)
            rval.right = ms.anchor_position.x;

        // lower boundary
        if (ms.anchor_position.y < rval.lower)
            rval.lower = ms.anchor_position.y;
    }

    return rval;
}   

bool 
ips::is_within_boundaries(point2d point, boundaries limits) const
{
    // left
    if (point.x < limits.left)
        return false;

    // upper
    if (point.y > limits.upper)
        return false;

    // right
    if (point.x > limits.right)
        return false;

    // lower
    if (point.y < limits.lower)
        return false;

    return true;
}