#pragma once
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

class Point2d
{
public:
	Point2d() {};
	Point2d(double x, double y);
	static double computing_distance(const Point2d& p1, const Point2d& p2);
	~Point2d();
	string point_print() const { stringstream ss; ss << "(" << this->x << ", " << this->y << ")"; return ss.str(); };
	double m_x() const { return this->x; }
	double m_y() const { return this->y; }
	static Point2d middle(const Point2d p1, const Point2d p2);
	static bool is_smaller(const Point2d& p1, const Point2d& p2); // returns true is p1 is to the left in the respect of p2
	bool operator== (const Point2d& p) const { return (this->x == p.x && this->y == p.y); };
	bool operator!= (const Point2d& p) const { return !(*this == p); };
private:
	double x;
	double y;
};

