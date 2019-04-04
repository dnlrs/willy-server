#include "Point2d.h"


Point2d::Point2d(double x, double y)
{
	this->x = x; this->y = y;
}


double Point2d::computing_distance(const Point2d& p1, const Point2d& p2)
{ 
	return sqrt(pow(p1.m_y() - p2.m_y(), 2) + pow(p1.m_x() - p2.m_x(), 2));
}

Point2d Point2d::middle(Point2d p1, Point2d p2)
{
	double x_midd = (p1.m_x() + p2.m_x()) / 2;
	double y_midd = (p1.m_y() + p2.m_y()) / 2;
	return Point2d(x_midd, y_midd);
}

bool Point2d::is_smaller(const Point2d& p1, const Point2d& p2)
{
	if (p1 == p2)
		return false;
	if (p1.m_x() == p2.m_x())
		return (p1.m_y() < p2.m_y());
	return (p1.m_x() < p2.m_x());
}

Point2d::~Point2d()
{
}
