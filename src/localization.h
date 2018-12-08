#pragma once

#include <cmath>
#include <memory>
#include <cstdarg>
#include <utility> // for swap
#include <vector>
#include "Point2d.h"
#include "Line2d.h"


#define C 0  // no shadowing effect: shadowing effect occurs when there are obstacles between sender and receiver
#define n 1.6 //2.7

using namespace std;


/* it uses the log normal path shadowing model:
		
		RSSI(d) = -10nlog(d) - C
		
	where log is the 10-th logarithmic scale
*/
double rssitod(int rssi)
{
	double exp = ((double)rssi + C)/(-10*n);
	return pow(10, exp);
	//return ( pow( 10, ((double)rssi - A)/(-10*n)) );
}


/* intersection of 3 circumferences:
	
	*-
		(x-x1)^2 + (y - y1)^2 = r1^2
		(x-x2)^2 + (y - y2)^2 = r2^2
		(x-x3)^2 + (y - y3)^2 = r3^2
	-*
	
	they can be transformed into:
	*-
		(-2x1 + 2x2)x + (-2y1 + 2y2)y = r1^2 - r2^2 - x1^2 + x2^2 - y1^2 + y2^2
		(-2x2 + 2x3)x + (-2y2 + 2y3)y = r2^2 - r3^2 - x2^2 + x3^2 - y2^2 + y3^2
	-*
	
	which has the form:
	*-
		Ax + By = C
		Dx + Ey = F
	-*
	
	so the solutions are:
	*-
		x = (CE - FB)/(EA - BD)
		y = (CD - AF)/(BD - AE)
	-*
*/
Point2d trilat(vector<pair<Point2d, int>> args)
{
	//if only 2 anchors it corresponds to the weighted loc method
	if(args.size() == 2)
		return weighted_loc(args);
	
	

}


Point2d weighted_loc(vector<pair<Point2d, int>> args)
{
	double totweight = 0,
		weighted_y = 0,
		weighted_x = 0;
	double curr_weight;

	for (pair<Point2d, int> p : args)
	{
		curr_weight = (double)1/rssitod(p.second); // TODO: try also 1/d^2
		weighted_x += p.first.m_x() * curr_weight;
		weighted_y += p.first.m_y() * curr_weight;
		totweight += curr_weight;
	}
	return Point2d((double)weighted_x/totweight, (double)weighted_y/totweight);
}


















