#include "point2d.h"
#include <cmath>

double compute_distance(const point2d& p1, const point2d& p2)
{ 
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

point2d compute_middle_point(const point2d p1, const point2d p2)
{
    return point2d((p1.x + p2.x)/2, (p1.y + p2.y)/2);
}

bool 
point2d::operator<(const point2d& other) const 
{

    if (x == other.x) {
        if (y == other.y) {
            return false;
        }
        return (y < other.y);
    }

    return (x < other.x);
}

bool 
point2d::operator==(const point2d& other) const 
{
    return (x == other.x && y == other.y);
}

bool
point2d::operator!=(const point2d& other) const 
{
    return (x != other.x || y != other.y);
}

std::string
point2d::str() const 
{
    return std::string(
        "(" + std::to_string(x) + ", " + std::to_string(y) + ")");
}