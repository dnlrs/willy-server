#include "connection_set.h"
#include "net_exception.h"
#include "socket_utils.h"

mac_addr
connection_set::get_anchor_mac(SOCKET in_socket)
{
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);
    return socket_to_mac[in_socket];
}

std::vector<SOCKET>
connection_set::get_open_sockets()
{
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);

    std::vector<SOCKET> rval;
    for (auto soc_pair : socket_to_mac)
        rval.push_back(soc_pair.first);

    return rval;
}

void
connection_set::add_connected_anchor(
    const SOCKET new_socket,
    const anchor new_anchor)
{
    if (new_socket == INVALID_SOCKET)
        throw net_exception("connection_set::add_connected_anchor: "
            "cannot add a new connected anchor with invalid socket");

    mac_addr anchor_mac = new_anchor.get_mac();

    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);

    if (anchors.find(anchor_mac) != anchors.end()) {
        debuglog("Newly connected anchor was connected previously");
        SOCKET previous_socket = mac_to_socket[anchor_mac];
        ::close_connection(&previous_socket);
    }

    anchors[anchor_mac]       = new_anchor;
    mac_to_socket[anchor_mac] = new_socket;
    socket_to_mac[new_socket] = anchor_mac;
}

void
connection_set::close_connection(SOCKET old_socket)
{
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);
    mac_addr old_anchor_mac = socket_to_mac[old_socket];

    if (anchors.find(old_anchor_mac) != anchors.end())
        anchors.erase(old_anchor_mac);

    if (mac_to_socket.find(old_anchor_mac) != mac_to_socket.end())
        mac_to_socket.erase(old_anchor_mac);

    if (socket_to_mac.find(old_socket) != socket_to_mac.end())
        socket_to_mac.erase(old_socket);

    ::close_connection(&old_socket);
}

void
connection_set::close_all_connections()
/*
 * Removing items from maps while iterating on them 
 * will result in access read violation error. That's
 * why the double cycle.
 */
{
    std::unique_lock<std::recursive_mutex> guard(anchors_rmtx);

    std::vector<SOCKET> connections_to_be_closed;
    for (auto dead_connection : socket_to_mac)
        connections_to_be_closed.push_back(dead_connection.first);

    for (auto dead_socket : connections_to_be_closed)
        close_connection(dead_socket);
}
