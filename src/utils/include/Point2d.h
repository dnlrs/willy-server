#ifndef POINT2D_H_INCLUDED
#define POINT2D_H_INCLUDED
#pragma once

#include <string>
#include <utility>

class point2d
{
public:
    point2d(double x_in = 0.0, double y_in = 0.0) : 
        x(x_in), y(y_in) {}
    point2d(std::pair<double, double> xy_coordinates) : 
        x(xy_coordinates.first), y(xy_coordinates.second) {}
    
    ~point2d() {}

    bool operator<(const point2d& other) const;
    bool operator==(const point2d& other) const;
    bool operator!=(const point2d& other) const;
    
    std::string to_string() const;

    friend double  compute_distance(const point2d& p1, const point2d& p2);
	friend point2d compute_middle_point(const point2d p1, const point2d p2);

public:
	double x;
	double y;
};

#endif // !POINT2D_H_INCLUDED