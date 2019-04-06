#ifndef DEALER_H_INCLUDED
#define DEALER_H_INCLUDED
#pragma once

#include "wwsadata.h"
#include "packet.h"
#include "anchor.h"
#include <map>
#include <memory>
#include <mutex>
#include <list>
#include <winsock2.h>
#include "Net_exception.h"
#include "Receiver.h"

#define SERVICE_PORT 27015 /* port used for the connection with the receivers */

/*
 * The dealer is the one who is in charge to handle the connection with the boards when the game starts
 * and whenever issues coming. Is the one who holds the listening Sockets and all the synchronization tools
 */
class Dealer
{
public:

    Dealer();

    // reads configuration file
    void init();
    void start();
    void stop();

private:
	// it setups the listening_socket for the server
	void setup_listening_socket();


	Dealer(vector<Receiver>& receivers) : listening_socket(INVALID_SOCKET), recvs(receivers), fatal_error(false){}
	~Dealer() { closesocket(listening_socket); }
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
	void close_listening() { closesocket(this->listening_socket); }
    //returns positions of all connected anchors
    std::map<uint64_t, Point2d> get_anchor_positions();



private:
    wwsadata data;

private:
	SOCKET listening_socket;
    //std::map<uint64_t, Point2d> anchor_positions;
    std::vector<anchor>      configured_anchors;
    std::map<SOCKET, anchor> connected_anchors;

	vector<Receiver>& recvs;
	mutex printMtx;
	mutex fatalErrMtx;
	boolean fatal_error; //set if one receiver thread is exited

    std::mutex recvs_mtx;
};

#endif // !DEALER_H_INCLUDED
