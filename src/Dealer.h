#pragma once
#include <winsock2.h>
#include <map>
#include <memory>
#include <mutex>
#include <list>
#include "Net_exception.h"
#include "Receiver.h"
#include "packet.h"

#define SERVICE_PORT 27015 /* port used for the connection with the receivers */

/*
 * The dealer is the one who is in charge to handle the connection with the boards when the game starts
 * and whenever issues coming. Is the one who holds the listening Sockets and all the synchronization tools
 */
class Dealer
{
public:
	Dealer(vector<Receiver>& receivers) : listenSocket(INVALID_SOCKET), recvs(receivers), fatal_error(false){}
	~Dealer() { closesocket(listenSocket); }
	// it setups the listenSocket for the server
	void setup_listeningS();
	// it waits for all the boards, accept their connection requests and initialize each receiver's socket
	void connect_to_all();
	// checks if the ip belongs to one of the expected boards
	int check_if_valid_board(const u_long& ip, const PMIB_IPNET_TABLE2& arpTable, const vector<Receiver>& receivers, const size_t nrecv);
	//listen on the listening socket for incoming connection requests. It grants the access only for authorized devices
	void accept_incoming_req();
	//returns the print mutex object
	mutex& getprintMtx() { return this->printMtx; }
	//it closes all the receivers socket and notify them to exit. Call it in order to close the server
	void notify_fatal_err();
	//it says if fatal_error is set
	boolean in_err();
	//closes the listening socket
	void close_listening() { closesocket(this->listenSocket); }
    //returns positions of all connected anchors
    std::map<uint64_t, Point2d> get_anchor_positions();

private:
	SOCKET listenSocket;
	vector<Receiver>& recvs;
	mutex printMtx;
	mutex fatalErrMtx;
	boolean fatal_error; //set if one receiver thread is exited
};

