#pragma once

#include <cmath>
#include <memory>
#include <cstdarg>
#include <utility> // for swap
#include <vector>
#include "Point2d.h"
#include "Line2d.h"


#define A -54
#define n 2.7

using namespace std;

double rssitod(int rssi)
{
	return ( pow( 10, ((double)rssi - A)/(-10*n)) );
}



Point2d localize(Point2d p1, int rssi1, Point2d p2, int rssi2)
{
	solutions s1, s2;
	double d1 = 0,
		  d2 = 0;
	if (rssi1 != 0)
		d1 = rssitod(rssi1);
	if (rssi2 != 0)
		d2 = rssitod(rssi2);


	assert(d1 >= 0 && d2 >= 0);
	if (!Point2d::is_smaller(p1, p2))
	{
		swap(p1, p2);
		swap(d1, d2);
	}
		
	Line2d line(p1, p2);
	s1 = line.walk_from(p1, d1);
	s2 = line.walk_from(p2, d2);

	// the 2 final points on which I will compute the middle point to estimate the host position
	shared_ptr<Point2d> finalp1, finalp2;
	// case: estimated position in between the 2 receivers
	if (Point2d::computing_distance(s1.pospoint, s2.negpoint) <= Point2d::computing_distance(s1.pospoint, s2.pospoint)
		&& Point2d::computing_distance(s1.pospoint, s2.negpoint) <= Point2d::computing_distance(s1.negpoint, s2.negpoint))
	{
		finalp1 = make_shared<Point2d>(s1.pospoint); // p1 must be the "smallest" point (the one on the left)
		finalp2 = make_shared<Point2d>(s2.negpoint); // p2 must be the "greatest" point (the one on the right)
	}
	else  // case: estimated position on the left of the "smallest" point
		if (Point2d::computing_distance(s1.negpoint, s2.negpoint) <= Point2d::computing_distance(s1.pospoint, s2.pospoint)
			&& Point2d::computing_distance(s1.negpoint, s2.negpoint) <= Point2d::computing_distance(s1.pospoint, s2.negpoint))
		{
			finalp1 = make_shared<Point2d>(s1.negpoint);
			finalp2 = make_shared<Point2d>(s2.negpoint);
		}
		else // case: estimated position on the right of the "greatest" point
		{
			finalp1 = make_shared<Point2d>(s1.pospoint);
			finalp2 = make_shared<Point2d>(s2.pospoint);
		}

	return Point2d::middle(*finalp1, *finalp2);
}
/*
Point2d localizen(vector<pair<Point2d, int>>& args)
{
	
	Point2d px = localize(args[0].first, args[0].second, args[1].first, args[1].second);
	if (args.size() - 2 == 0)
		return px;
	int resultingrssi = (args[0].second + args[1].second)/2;
	args.erase(args.begin()); // erase r1
	args.erase(args.begin()); //erase r2
	args.push_back(make_pair(px, resultingrssi));
	px = localizen(args);
	return px;
}*/

Point2d localizen(vector<pair<Point2d, int>> args)
{
	double totweight = 0,
		weighted_y = 0,
		weighted_x = 0;
	double curr_weight;

	for (pair<Point2d, int> p : args)
	{
		curr_weight = (double)1/rssitod(p.second);
		weighted_x += p.first.m_x() * curr_weight;
		weighted_y += p.first.m_y() * curr_weight;
		totweight += curr_weight;
	}
	return Point2d((double)weighted_x/totweight, (double)weighted_y/totweight);
}
