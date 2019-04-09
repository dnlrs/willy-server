#ifndef DEALER_H_INCLUDED
#define DEALER_H_INCLUDED
#pragma once

#include "wwsadata.h"
#include "packet.h"
#include "anchor.h"
#include "cfg.h"
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
    ~Dealer();

    /* reads configuration file */
    void init(std::string conf_file);
    void start();
    void stop();

private:
	/* Setups the listening_socket for the server */
	void setup_listening_socket();

    /* Connects to all anchors */
    void connect_all_anchors();

    /* Manages an incoming anchor connection request
     * 
     * This may block indefinitively if no anchor 
     * tries to connect. 
     * In the anchor returned only the mac address
     * and the IP address are valid, position will 
     * be (0,0)
     */
    anchor connect_anchor(SOCKET* rsocket);

    /* Sends an ACK to a newly connected anchor */
    void send_connection_ack(
        const SOCKET anchor_socket);
    
    /* sets the SO_KEEPALIVE option */
    void set_keepalive_option(
        const SOCKET anchor_socket);

    /* Adds a new connected anchor
     * 
     * If the new anchor had already a previous opened 
     * connection on another socket, that connection is 
     * is closed and removed and the new connection is 
     * saved
     */
    void add_connected_anchor(
        const SOCKET new_socket, 
        const anchor new_anchor);
    
    /* Removes a previously connected anchor 
     *
     * If the old socket is not invalid socket, then
     * this function takes also care about shutting down
     * and closing the socket. If any of these operations
     * go wrong throws net_exception.
     */
    void remove_connected_anchor(
        const uint64_t anchor_mac);

	Dealer(vector<Receiver>& receivers) : listening_socket(INVALID_SOCKET), recvs(receivers), fatal_error(false){}

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
    cfg::configuration context;
	SOCKET listening_socket = INVALID_SOCKET;

private:
    std::map<uint64_t, anchor>  anchors;        // {mac, anchor}
    std::map<uint64_t, SOCKET>  mac_to_socket;  // {mac, socket}
    std::map<SOCKET, uint64_t>  socket_to_mac;  // {socket, mac}
    std::recursive_mutex        anchors_rmtx;


	vector<Receiver>& recvs;
	mutex printMtx;
	mutex fatalErrMtx;
	boolean fatal_error; //set if one receiver thread is exited

    std::mutex recvs_mtx;
};

#endif // !DEALER_H_INCLUDED
