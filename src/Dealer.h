#pragma once
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <map>
#include <memory>
#include <mutex>
#include "Net_exceptions.h"
#include "Receiver.h"
#include "packet.h"

#define SERVICE_PORT 27015 /* port used for the connection with the receivers */

/*
 * The dealer is the one who is in charge to handle the connection with the boards when the game starts
 * and whenever issues coming. Is the one who holds the listening Sockets
 */
class Dealer
{
public:
	Dealer(vector<Receiver>& receivers) : listenSocket(INVALID_SOCKET), recvs(receivers){}
	~Dealer() { closesocket(listenSocket); }
	// it setups the listenSocket for the server
	void setup_listeningS();
	// it waits for all the boards, accept their connection requests and initialize each receiver's socket
	void connect_to_all();
	// checks if the ip belongs to one of the expected boards
	int check_if_valid_board(const u_long& ip, const PMIB_IPNET_TABLE2& arpTable, const vector<Receiver>& receivers, const size_t nrecv);
	//listen on the listening socket for incoming connection requests. It grants the access only for authorized devices
	void accept_incoming_req();

	mutex& getprintMtx() { return this->printMtx; }

private:
	SOCKET listenSocket;
	vector<Receiver>& recvs;
	mutex printMtx;
};

