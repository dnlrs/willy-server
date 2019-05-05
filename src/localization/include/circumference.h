#ifndef CIRCUMFERENCE_H_INCLUDED
#define CIRCUMFERENCE_H_INCLUDED
#pragma once

#include "point2d.h"
#include <cmath>
#include <vector>

class circumference
{
public:
    circumference(
        point2d center_in   = point2d(),
        double  radius_in = 0) :
        center(center_in), radius(radius_in) {}

    bool operator==(const circumference& other) const;

    /* Returns false if there are no intersection points */
    bool get_intersection_points(
        const circumference& other,
        std::vector<point2d>& rval) const;

private:
    bool intersects(const circumference& other) const;

public:
    point2d center;
    double  radius;
};

#endif // !CIRCUMFERENCE_H_INCLUDED
