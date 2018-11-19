#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <stdint.h>
#include "packet.h"
#include "Point2d.h"
#include "Net_exceptions.h"

/* 
 * This is the class that represents a single receiver,
 * its information and the socket to contact it
 */
using namespace std;

class Receiver
{
public:
	Receiver(mac_t mac, double x, double y) : loc(x, y), mac(mac), sock(INVALID_SOCKET), in_error(false) {}
	~Receiver();
	mac_t m_mac() const { return this->mac; }
	Point2d m_loc() const { return this->loc; }
	double m_x() const { return this->loc.m_x(); }
	double m_y() const { return this->loc.m_y(); }
	// receives a struct packet from this receiver 
	PACKET_T operator() ();
	bool operator==(Receiver r) { return this->mac.compacted_mac == r.mac.compacted_mac; }
	// check if the socket is still connected
	boolean has_valid_socket();
private:
	mac_t mac;
	Point2d loc;
	SOCKET sock; /* socket for this receiver */
	boolean in_error; // Has this receiver raised an exception ?
	void set_sock(const SOCKET& s) { this->sock = s; }
	SOCKET m_sock() const { return this->sock; };

	friend class Dealer; /* the dealer can access the set_sock to fix the socket in case of malfunctions */
};

