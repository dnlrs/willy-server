#include "circumference.h"

bool circumference::operator==(const circumference& other) const
{
    return ((center == other.center) && (radius == other.radius));
}

bool circumference::intersects(const circumference& other) const
/*
 * Distance between centers C1 and C2 is calculated as
 *      C1C2 = sqrt((x1 - x2)^2 + (y1 - y2)^2)
 * which becomes
 *      C1C2^2 = (x1 - x2)^2 + (y1 - y2)^2
 *
 * If C1C2 == R1 + R2
 *      Circle A and B are touch to each other.
 * If C1C2 > R1 + R2
 *      Circle A and B are not touch to each other.
 * If C1C2 < R1 + R2
 *      Circle intersects each other.
 */
{
    double dx = center.x - other.center.x; // horizontal distance between centers
    double dy = center.y - other.center.y; // vertical distance between centers

    double r_sum = radius + other.radius;  // sum of circumferences radiuses

    double centers_distance2 = ((dx*dx) + (dy*dy));
    double max_distance2 = (r_sum * r_sum);

    if (centers_distance2 > max_distance2)
        return false;

    return true;
}

bool
circumference::get_intersection_points(
    const circumference& other,
    std::vector<point2d>& rval) const
/*
 * ref: http://paulbourke.net/geometry/circlesphere/
 * 
 * (x0, y0) center of first circumference
 * r0       radius of first circumference
 * (x1, y1) center of second circumference
 * r1       radius of second circumference
 *
 * P2       point of intersection between line passing by centers and line
 *          passing by intersection points
 * (x2, y2) coordinates of P2
 * a        distance between first center and P2
 * h        distance between P2 and any of the intersection points
 *
 * Ox       horizontal offset of intersection points from P2
 * Oy       vertical offset of intersection points from P2
 *
 * (poi_x0, poi_y0) first point of intersection
 * (poi_x1, poi_y1) second point of intersection
 */
{
    if (intersects(other) == false)
        return false;

    double x0 = center.x;
    double y0 = center.y;
    double r0 = radius;

    double x1 = other.center.x;
    double y1 = other.center.y;
    double r1 = other.radius;

    double dx = x1 - x0;            // horizontal distance between centers
    double dy = y1 - y0;            // vertical distance between centers
    double d  = std::hypot(dx, dy); // distance between centers - sqrt(dx^2 + dy^2)

    double a = ((r0 * r0) - (r1 * r1) + (d * d)) / (2.0 * d);
    double x2 = x0 + (dx * a / d);
    double y2 = y0 + (dy * a / d);

    double h = std::hypot(r0, a); // sqrt(r0^2 + a^2)

    double Ox = -dy * (h / d);
    double Oy = dx * (h / d);

    double poi_x0 = x2 + Ox;
    double poi_y0 = y2 + Oy;

    double poi_x1 = x2 - Ox;
    double poi_y1 = y2 - Oy;

    point2d poi0(poi_x0, poi_y0);
    point2d poi1(poi_x1, poi_y1);

    rval.clear();
    rval.push_back(poi0);

    if ((poi0 == poi1) == false)
        rval.push_back(poi1);

    return true;
}
