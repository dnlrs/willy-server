#pragma once
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <map>
#include <memory>
#include "Net_exceptions.h"
#include "Receiver.h"
#include "packet.h"

#define SERVICE_PORT 27015 /* port used for the connection with the receivers */

/*
 * The dealer is the one who is in charge to handle the connection with the boards when the game starts
 * and whenever issues coming. Is the one who hold the listening Socket
 */
class Dealer
{
public:
	Dealer(vector<Receiver>& receivers) : listenSocket(INVALID_SOCKET), recvs(receivers) {}
	~Dealer() { closesocket(listenSocket); }
	// it setup the listenSocket for the server
	void setup_listening();
	// it waits for all the boards, accept their connection requests and initialize each receiver's socket
	void setup_all();
	// check if the ip belongs to one of the expected boards
	int check_if_valid_board(const u_long& ip, const PMIB_IPNET_TABLE2& arpTable, const vector<Receiver>& receivers, const size_t nrecv);
	// it tries to reconnect to a specific receiver
	void reconnecting(Receiver& r);
	// sends the ACK to all the boards and set the lastSynch with the actual local time
	void synch();
	// it returns the local time + offset (in seconds) in unix time
	time_t synchToUnix(UINT16 off);

private:
	SOCKET listenSocket;
	map<string, SOCKET> pendingRequests; // <board's mac, socket>
	vector<Receiver>& recvs;
	SYSTEMTIME lastSynch;
	LONGLONG FileTime_to_POSIX(FILETIME ft);
};

