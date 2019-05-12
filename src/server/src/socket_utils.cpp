#include "logger.h"
#include "net_exception.h"
#include "socket_utils.h"
#include "sock_exception.h"
#include "utils.h"
#include <cassert>
#include <mstcpip.h>

using namespace std::this_thread;
using namespace std::chrono;

using std::vector;
using std::to_string;

SOCKET setup_listening_socket(unsigned short listening_port)
{
    SOCKET listening_socket = INVALID_SOCKET;

    // create socket
    listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listening_socket == INVALID_SOCKET)
        throw net_exception("setup_listening_socket: creation failed\n" +
            wsa_etos(WSAGetLastError()));

    // make listening socket non-blocking
    set_non_blocking_socket(listening_socket);

    // local endpoint parameters for listening socket
    struct sockaddr_in service;
    memset(&service, 0, sizeof(service));

    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(listening_port);
    service.sin_family = AF_INET;

    // Bind socket
    int err = SOCKET_ERROR;
    err = ::bind(
        listening_socket, (const sockaddr*)&service, (int) sizeof(service));
    if (err == SOCKET_ERROR) {
        int wsa_err = WSAGetLastError();
        closesocket(listening_socket);
        listening_socket = INVALID_SOCKET;
        throw net_exception("setup_listening_socket: binding failed\n" +
            wsa_etos(wsa_err));
    }

    // start listening for incoming connection requests
    err = ::listen(listening_socket, SOMAXCONN);
    if (err == SOCKET_ERROR) {
        int wsa_err = WSAGetLastError();
        closesocket(listening_socket);
        listening_socket = INVALID_SOCKET;
        throw net_exception("setup_listening_socket: listeing failed\n" +
            wsa_etos(wsa_err));
    }

    return listening_socket;
}

SOCKET setup_for_broadcasting()
{
    SOCKET rval = INVALID_SOCKET;

    rval = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (rval == INVALID_SOCKET)
        throw net_exception("setup_for_broadcasting: socket failed\n" +
            wsa_etos(WSAGetLastError()));

    int err = 1;
    bool value = true;
    err = setsockopt(rval, SOL_SOCKET, SO_BROADCAST, (char*)&value, sizeof(int));
    if (err == SOCKET_ERROR)
        throw net_exception("setup_for_broadcasting: setsockopt failed\n" +
            wsa_etos(WSAGetLastError()));

    return rval;
}

void
set_keepalive_option(
    const SOCKET anchor_socket)
{
    if (anchor_socket == INVALID_SOCKET)
        throw sock_exception(
            "Cannot set option on invalid socket", anchor_socket);

    int err = SOCKET_ERROR;

#ifdef _WIN32

    u_long bytes_returned = 0;
    tcp_keepalive keepalive_vals;
    keepalive_vals.onoff             = 1;
    keepalive_vals.keepalivetime     = default_keepalive_time_ms;
    keepalive_vals.keepaliveinterval = default_keepalive_interval_ms;

    err = WSAIoctl(
        anchor_socket,          /* socket descriptor */
        SIO_KEEPALIVE_VALS,     /* io control code */
        &keepalive_vals,        /* tcp_keepalive structure */
        sizeof(keepalive_vals), /* size of tcp_keepalive struct */
        NULL, 0,                /* output buffer and its size */
        &bytes_returned,        /* nr. of bytes returned */
        NULL, NULL);            /* overlapped struct, completion routine */

#else

    uint8_t opt_value = 1;
    err = setsockopt(anchor_socket, SOL_SOCKET, SO_KEEPALIVE,
        (const char*)&opt_value, sizeof(bool));

#endif // !_WIN32

    if (err == SOCKET_ERROR)
        throw sock_exception(
            "Setting socket option KEEPALIVE failed\n", WSAGetLastError());
}

void
set_non_blocking_socket(
    const SOCKET socket_in)
{
    if (socket_in == INVALID_SOCKET)
        throw sock_exception(
            "Cannot set non blocking an invalid socket", socket_in);

    u_long mode = 1;
    int err = ioctlsocket(socket_in, FIONBIO, &mode);

    if (err == SOCKET_ERROR)
        throw sock_exception(
            "Cannot set non-blocking mode on socket\n", WSAGetLastError());
}

void
close_connection(SOCKET* psocket)
{
    if (psocket == nullptr)
        return;

    if (*psocket == INVALID_SOCKET)
        return;

    if (shutdown(*psocket, SD_SEND) == SOCKET_ERROR)
        debuglog("shutdown socket error or UDP socket (no harm)\n",
            wsa_etos(WSAGetLastError()));

    if (closesocket(*psocket) == SOCKET_ERROR)
        debuglog("closesocket error: ", wsa_etos(WSAGetLastError()));

    *psocket = INVALID_SOCKET;
}

uint32_t
read_sized_message(
    vector<uint8_t>& rval,
    const SOCKET raw_socket)
{
    if (raw_socket == INVALID_SOCKET)
        throw net_exception("read_sized_message: "
            "socket cannot be an INVALID_SOCKET");

    uint8_t buf[default_buffer_size];
    memset(&buf[0], 0, default_buffer_size);

    uint32_t read_bytes = 0;
    uint32_t msg_length = 4; // first 4 bytes

    // read message size
    read_bytes = read_n(&(buf[0]), msg_length, raw_socket);
    assert(read_bytes == 4);

    msg_length = ntohl(*((uint32_t*)&buf[0]));
    if (msg_length > default_buffer_size) {
        throw net_exception("read_sized_message: "
            "msg length (" + to_string(msg_length) + ") " +
            "is greater than max buffer size.");
    }

    // reset first 4 bytes
    *((uint32_t*)&buf[0]) = 0;

    // read actual message
    read_bytes = read_n(&(buf[0]), msg_length, raw_socket);
    assert(read_bytes == msg_length);

    rval = std::move(vector<uint8_t>(buf, buf + msg_length));
    return msg_length;
}

uint32_t
read_n(
    uint8_t* const dst_buffer,
    const uint32_t msg_length,
    const SOCKET raw_socket)
{
    uint8_t* pmsg      = dst_buffer;
    int32_t left_bytes = msg_length;
    int32_t read_bytes = 0;

    long attempts = default_wouldblock_attempts;
    while (left_bytes > 0 && attempts > 0) {
        read_bytes = ::recv(raw_socket, (char*)pmsg, left_bytes, 0);

        if (read_bytes == SOCKET_ERROR) {
            int wsa_err = WSAGetLastError();

            if (wsa_err == WSAEWOULDBLOCK) {
                sleep_for(milliseconds(
                    default_wouldblock_sleep));
                attempts--;
                continue;
            }
            else {
                throw sock_exception("read_n: "
                    "recv failed\n", wsa_err, raw_socket);
            }
        }

        if (read_bytes == 0) {
            throw sock_exception("read_n: "
                "socket has been closed by peer", raw_socket);
        }

        pmsg       += read_bytes;
        left_bytes -= read_bytes;
        attempts    = default_wouldblock_attempts;
    }

    if (attempts <= 0)
        throw sock_exception("read_n: "
            "connection transmission taking too long to complete",
            raw_socket);

    return (uint32_t)(pmsg - dst_buffer);
}

void bcast_udp_message(
    char* msg,
    uint32_t msg_size,
    uint16_t dst_port,
    SOCKET raw_socket)
{
    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));

    target.sin_addr.s_addr = INADDR_BROADCAST;
    target.sin_port   = htons(dst_port);
    target.sin_family = AF_INET;

    int message_size = msg_size;
    int sent_bytes = 0;
    char* pmsg     = msg;
    while (message_size > 0) {
        sent_bytes = ::sendto(raw_socket, pmsg, message_size, NULL,
            (const sockaddr*)&target, sizeof(target));

        if (sent_bytes == SOCKET_ERROR)
            throw net_exception("bcast_udp_message: sendto failed to send message" +
                wsa_etos(WSAGetLastError()));

        pmsg         += sent_bytes;
        message_size -= sent_bytes;
    }
}
