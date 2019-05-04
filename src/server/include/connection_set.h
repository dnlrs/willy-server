#ifndef CONNECTION_SET_H_INCLUDED
#define CONNECTION_SET_H_INCLUDED
#pragma once

#include "anchor.h"
#include "mac_addr.h"
#include "winsock2.h"
#include <map>
#include <mutex>

/*
 * Forward declaration so it can be friend and access private members.
 * -> Do NOT access private data members directly from dealer <-
 */
class dealer;

class connection_set
{
public:
    mac_addr            get_anchor_mac(SOCKET in_socket);
    std::vector<SOCKET> get_open_sockets();

private:
    friend class dealer;

    /* Adds a new connected anchor
    *
    * If the new anchor had already a previous opened
    * connection on another socket, that connection is
    * closed and removed and the new connection is saved
    */
    void add_connected_anchor(
        const SOCKET new_socket,
        const anchor new_anchor);

    /* Gracefully closes a connections and removes anchor's data */
    void close_connection(SOCKET old_socket);

    /* Closes all opened connections removing all anchors' data */
    void close_all_connections();

private:
    std::recursive_mutex anchors_rmtx;         // protects all maps
    std::map<mac_addr, anchor> anchors;        // {mac, anchor}
    std::map<mac_addr, SOCKET> mac_to_socket;  // {mac, socket}
    std::map<SOCKET, mac_addr> socket_to_mac;  // {socket, mac}
};

#endif //!CONNECTION_SET_H_INCLUDED
