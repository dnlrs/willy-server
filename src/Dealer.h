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
#include "net_exception.h"
#include "receiver.h"

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
    
    void finish();

    uint64_t get_anchor_mac(SOCKET in_socket);
    std::vector<SOCKET> get_opened_sockets();
    void notify_anchor_disconnected(SOCKET dead_socket);
    void notify_fatal_error();


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

    //std::map<uint64_t, Point2d> get_anchor_positions();



    void service();


    wwsadata data;

    cfg::configuration context;
	SOCKET listening_socket = INVALID_SOCKET;

    std::map<uint64_t, anchor>  anchors;        // {mac, anchor}
    std::map<uint64_t, SOCKET>  mac_to_socket;  // {mac, socket}
    std::map<SOCKET, uint64_t>  socket_to_mac;  // {socket, mac}
    std::recursive_mutex        anchors_rmtx;

    Receiver collector;

};

#endif // !DEALER_H_INCLUDED
