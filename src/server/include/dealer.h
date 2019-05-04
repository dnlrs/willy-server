#ifndef DEALER_H_INCLUDED
#define DEALER_H_INCLUDED
#pragma once

#include "anchor.h"
#include "cfg.h"
#include "connection_set.h"
#include "packet.h"
#include "mac_addr.h"
#include "net_exception.h"
#include "receiver.h"
#include "wwsadata.h"
#include <atomic>
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")   //winSock library
#pragma comment(lib, "iphlpapi.lib") //windows IP Helper API

/* listening port for incoming connections */
#define SERVICE_PORT 27015

constexpr long int dealer_waiting_time_ms = 200;
constexpr int      dealer_max_accept_attempts = 10;

/*
 * The dealer is the one who is in charge to handle the connection with the
 * boards when the game starts and whenever issues coming.
 * Is the one who holds the listening socket and the connected sockets
 */
class dealer
{
public:

    dealer();
    ~dealer();

    /* Inits system configuration and listening socket
     *
     * Creates the system configuration by reading the
     * configuration file and initializes the listening
     * socket
     */
    void init(std::string conf_file);

    /* Asks the receiver to start and deploys the dealer thread */
    void start();

    /* Asks the receiver to stop and stops the dealer thread (non blocking) */
    void stop();

    /* Asks the receiver to finish, rejoins the dealer thread
     * and closes all open connections (blocking)
     */
    void finish();

    /* Calls stop() then finish() (blocking) */
    void shutdown();

    /* Closes the connection server side and removes it from active connections */
    void notify_anchor_disconnected(SOCKET dead_socket);

    /* Stops the server *without joining threads* */
    void notify_fatal_error();

private:
    /* Dealer thread function
     *
     * Accepts new connection requests from anchors until all anchors
     * are connected and when the receiver notifies that a connection
     * went down.
     */
    void service();

    /* Manages an incoming anchor connection request
     *
     * This may block indefinitively if no anchor
     * tries to connect.
     * In the anchor returned only the mac address
     * and the IP address are valid, position will
     * be (0,0)
     * */
    anchor connect_anchor(SOCKET* rsocket);

    /* Sends an ACK to a newly connected anchor */
    void send_connection_ack(
        const SOCKET anchor_socket);

private:
    wwsadata data;

    SOCKET listening_socket = INVALID_SOCKET;

    /* Data about all connected anchors and connections */
    std::shared_ptr<connection_set> connections = nullptr;

    /*
     * This is the number of lost connections or dead anchors.
     * While this variable is > 0 received packets will not be
     * saved and the dealer will be listening for incoming
     * connection requestes from anchors.
     */
    std::shared_ptr<std::atomic_int> dead_anchors = nullptr;

    /*
     * Reference to receiver thread
     * Note that this is also the control variable to check
     * if server is running, so when the server is not running
     * this shall be nullptr
     */
    std::unique_ptr<receiver> preceiver = nullptr;

    /* thread controlling variables (dealer's thread only) */
    std::atomic_bool        stop_working = false;
    std::mutex              dealer_mtx;
    std::condition_variable dealer_cv;

    /* reference to the actual dealer thread */
    std::thread dealer_thread;

    /* system wide configuration */
    std::shared_ptr<cfg::configuration> context = nullptr;
};

#endif // !DEALER_H_INCLUDED
