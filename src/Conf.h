#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "packet.h" //for MAC_LENGTH define
#include "Conf_exception.h"

/* This class read the conf file token by token (receiver by receiver) */

using namespace std;

/* file format :		(the MAC should be in the colon ':' form)
 *		#receivers
 *		MAC(i) X(i) Y(i) 
 *		. . .
*/

class Conf
{
public:
	Conf() : receiver_n(0), actual_recv(0), finished(false) {}
	~Conf();
	void load_conf(const string filename); /* open the file and reads once '#receivers' */
	void operator() (); /* reads parameter of the next board */
	int m_recvn() const { return this->receiver_n; }
	int m_actual_recv() const { return this->actual_recv; }
	mac_t m_mac() const { return this->mac_addr; }
	float m_x() const { return this->curr_x; }
	float m_y() const { return this->curr_y; }
	void next_line();
private:	
	int receiver_n; /* read once in the constructor */
	int actual_recv;
	mac_t mac_addr;
	float curr_x;
	float curr_y;
	ifstream file;
	bool finished;

};

