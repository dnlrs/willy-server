#include "dealer.h"
#include "utils.h"
#include "socket_utils.h"
#include "wARPtable.h"
#include <ws2tcpip.h>
#include <mstcpip.h>


Dealer::Dealer()
{

}

Dealer::~Dealer()
{

}


void Dealer::init(std::string conf_file)
{
    // read configuration
    context.import_configuration(conf_file, true);
    if (context.get_anchors_number() == 0)
        throw net_exception("No anchors found in configuration file");

    // setup listening socket
    setup_listening_socket();
}


void Dealer::start()
{
    // start receiver thread -- receiver::service
        // starts doing select on available sockets
        // if dealer says to ignore packets, ignores packets
    // start worker threads -- worker::service
        // waits on queue for packets to deserialize
        // deserializes packets, localizes devices, inserts packets into database
    // start dealer thread -- service

    // "go" to receiver thread
    // connect all anchors
    // connect_all_anchors();
}


void Dealer::stop()
{

}

void Dealer::setup_listening_socket()
{
    if (listening_socket != INVALID_SOCKET)
        throw net_exception("listening socket should be invalid");

	// Create a SOCKET for listening for incoming connection requests
	listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listening_socket == INVALID_SOCKET) 
        throw net_exception(
            "listening_socket creation fail\n" + wsa_etos(WSAGetLastError()));

    // set non blocking socket - TODO: make it unblocking and add max attempts in connect_anchor()
    //set_non_blocking_socket(listening_socket);
	
	// local endpoint parameters for listening socket (addr. family, IP, port)
	struct sockaddr_in service;
    memset(&service, 0, sizeof(service));

	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port        = htons(SERVICE_PORT);
	service.sin_family      = AF_INET;

    // Bind socket
    int err = SOCKET_ERROR;
    err = ::bind(listening_socket, (const sockaddr*) &service, sizeof(service));
	if (err == SOCKET_ERROR) {
        closesocket(listening_socket);
		listening_socket = INVALID_SOCKET;
        throw net_exception(
            "listening_socket bind fail\n" + wsa_etos(WSAGetLastError()));
	}
	
	// Listen for incoming connection requests
    err = ::listen(listening_socket, SOMAXCONN);
	if (err == SOCKET_ERROR) {
		closesocket(listening_socket);
		listening_socket = INVALID_SOCKET;
        throw net_exception(
            "listening_socket liste fail\n" + wsa_etos(WSAGetLastError()));
	}
}

void Dealer::connect_all_anchors()
{
    int anchors_number = context.get_anchors_number();
    bool rs = false;

    debuglog("Waiting for ", anchors_number, " to connect...");
    while (anchors_number > 0) {
        
        SOCKET new_socket = INVALID_SOCKET;
        anchor new_anchor = connect_anchor(&new_socket);

        std::pair<double, double> new_anchor_position(0, 0);
        rs = context.get_anchor_position(
                            new_anchor.get_mac(), new_anchor_position);
        if (rs == true) {
            // anchor was found in configuration file
            new_anchor.set_position(new_anchor_position);
            add_connected_anchor(new_socket, new_anchor);

            anchors_number--;
            debuglog("New anchor connected: ", mac_int2str(new_anchor.get_mac()));
        }
        else {
            throw net_exception("Anchor " + mac_int2str(new_anchor.get_mac()) + 
                                " was not found in configuration file");
        }

    }
}

void 
Dealer::add_connected_anchor(
    const SOCKET new_socket, 
    const anchor new_anchor)
{
    if (new_socket == INVALID_SOCKET)
        throw net_exception(
            "Cannot add a new connected anchor with invalid socket");
    
    uint64_t anchor_mac = new_anchor.get_mac();
    
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);
    
    if (anchors.find(anchor_mac) != anchors.end()) {
        debuglog("Newly connected anchor was connected previously");
        remove_connected_anchor(anchor_mac);
    }

    anchors[anchor_mac]       = new_anchor;
    mac_to_socket[anchor_mac] = new_socket;
    socket_to_mac[new_socket] = anchor_mac;

    // notify receiver that a new anchor connected
    collector.notify_anchor_connected();
}

void
Dealer::remove_connected_anchor(
    const uint64_t anchor_mac)
{
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);
    
    SOCKET old_socket = mac_to_socket[anchor_mac];
    
    if (anchors.find(anchor_mac) != anchors.end())
        anchors.erase(anchor_mac);
    
    if (mac_to_socket.find(anchor_mac) != mac_to_socket.end())
        mac_to_socket.erase(anchor_mac);
    
    if (socket_to_mac.find(old_socket) != socket_to_mac.end())
        socket_to_mac.erase(old_socket);

    if (old_socket != INVALID_SOCKET) {
        if (shutdown(old_socket, SD_SEND) == SOCKET_ERROR)
            debuglog(
                "shutdown socket error or UDP socket (no harm)\n",
                wsa_etos(WSAGetLastError()));

        if (closesocket(old_socket) == SOCKET_ERROR)
            debuglog("closesocket error: ", wsa_etos(WSAGetLastError()));
    }
}

anchor 
Dealer::connect_anchor(SOCKET* rsocket)
{
    if (listening_socket == INVALID_SOCKET)
        throw net_exception(
            "Cannot connect anchor because listening socket is invalid");

    *rsocket = INVALID_SOCKET;

    // accept new connection
    struct sockaddr_in anchor_sa;
    int anchor_sa_size = sizeof(anchor_sa);
    memset(&anchor_sa, 0, anchor_sa_size);

    SOCKET new_socket = 
        ::accept(listening_socket, (sockaddr*) &anchor_sa, &anchor_sa_size);

    if (new_socket == INVALID_SOCKET)
        throw net_exception("Accept failed while connecting an anchor\n" + 
            wsa_etos(WSAGetLastError()));
    
    // get anchor mac
    wARPtable arp_table;
    uint64_t new_mac = arp_table.get_mac_from_ip(anchor_sa.sin_addr.s_addr);

    // set socket option SO_KEEPALIVE
    set_keepalive_option(new_socket);

    // set non-blocking socket
    set_non_blocking_socket(new_socket);

    // send connection ack
    send_connection_ack(new_socket);

    // return new socket and new anchor
    *rsocket = new_socket;
    return anchor(new_mac, anchor_sa.sin_addr.s_addr);
}




void 
Dealer::send_connection_ack(const SOCKET anchor_socket)
{
    if (anchor_socket == INVALID_SOCKET)
        throw net_exception("Cannot send connection ack on invalid socket");

    uint32_t ack = 1;
    uint32_t left_bytes = sizeof(ack);
    const char *pbuf = (const char *) &ack;
    
    while (left_bytes > 0) {
        uint32_t sent_bytes = 
            ::send(anchor_socket, pbuf, left_bytes, 0);

        if (sent_bytes == SOCKET_ERROR)
            throw net_exception("Failed to send connection ACK\n" + 
                                wsa_etos(WSAGetLastError()));

        pbuf       += sent_bytes;
        left_bytes -= sent_bytes;
    }
}

uint64_t
Dealer::get_anchor_mac(SOCKET in_socket)
{
    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);
    // TODO: what if wrong socket in?
    return socket_to_mac[in_socket];
}

std::vector<SOCKET> 
Dealer::get_opened_sockets()
{
    std::vector<SOCKET> rval;

    std::lock_guard<std::recursive_mutex> guard(anchors_rmtx);

    for (auto soc_pair : socket_to_mac)
        rval.push_back(soc_pair.first);

    return rval;
}

void
Dealer::notify_anchor_disconnected(SOCKET dead_socket)
{
    // TODO: wake-up thread
}

void
Dealer::notify_fatal_error()
{
    // TODO: close everything
}

//std::map<uint64_t, Point2d> 
//Dealer::get_anchor_positions()
//{
//    std::map<uint64_t, Point2d> rval;
//
//    std::lock_guard<std::mutex> guard(recvs_mtx);
//
//    for (Receiver const &r: recvs)
//        rval[r.m_mac().compacted_mac] = r.m_loc();
//    
//    return rval;
//}