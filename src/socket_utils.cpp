#include "socket_utils.h"
#include "net_exception.h"
#include "utils.h"
#include <cassert>
#include <mstcpip.h>


void
set_keepalive_option(
    const SOCKET anchor_socket)
{
    if (anchor_socket == INVALID_SOCKET)
        throw net_exception("Cannot set option on invalid socket");

    int err = SOCKET_ERROR;

#ifdef _WIN32

    u_long bytes_returned = 0;
    tcp_keepalive keepalive_vals;

    keepalive_vals.onoff = 1;
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
        throw net_exception("Setting socket option KEEPALIVE failed\n" +
            wsa_etos(WSAGetLastError()));
}


void
set_non_blocking_socket(
    const SOCKET socket_in)
{
    if (socket_in == INVALID_SOCKET)
        throw net_exception("Cannot set non blocking an invalid socket");

    int err = SOCKET_ERROR;
    u_long mode = 1;
    err = ioctlsocket(socket_in, FIONBIO, &mode);

    if (err == SOCKET_ERROR)
        throw net_exception("Cannot set non-blocking mode on socket\n" +
            wsa_etos(WSAGetLastError()));
}


uint32_t 
read_sized_message(
    std::vector<uint8_t>& rval,
    const SOCKET raw_socket)
{
    if (raw_socket == INVALID_SOCKET)
        throw net_exception(
            "read_sized_message: parameter must be valid socket");
    
    uint8_t buf[default_buffer_size];
    memset(&buf, 0, default_buffer_size);

    uint32_t read_bytes = 0;
    uint32_t msg_length = 4; // first 4 bytes
    
    // read message size
    read_bytes = read_n(&(buf[0]), msg_length, raw_socket);
    assert(read_bytes == 4);

    uint32_t msg_length = ntohl((uint32_t)buf[0]);
    if (msg_length > default_buffer_size)
        throw net_exception(
            "read_sized_message: "
            "message length is greater than buffer size");
    memset(&buf, 0, 4);

    // read actual message
    read_bytes = read_n(&(buf[0]), msg_length, raw_socket);
    assert(read_bytes == msg_length);

    rval = std::move(std::vector<uint8_t>(buf, buf + msg_length));
    return msg_length;
}


uint32_t 
read_n(
    uint8_t* const dst_buffer,
    const uint32_t msg_length, 
    const SOCKET raw_socket)
{
    uint8_t* pmsg       = dst_buffer;
    uint32_t left_bytes = msg_length;
    uint32_t read_bytes = 0;

    long attempts = default_wouldblock_attempts;

    while (left_bytes > 0 && attempts > 0) {

        read_bytes = recv(raw_socket, (char*)pmsg, left_bytes, 0);

        if (read_bytes == SOCKET_ERROR) {
            int wsa_err = WSAGetLastError();

            if (wsa_err == WSAEWOULDBLOCK) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(default_wouldblock_sleep));
                attempts--;
            }
            else {
                throw net_exception("read_n: recv failed\n" + wsa_etos(wsa_err));
            }
        }
        else {
            pmsg       += read_bytes;
            left_bytes -= read_bytes;
            attempts    = default_wouldblock_attempts;
        }
    }

    return (pmsg - dst_buffer);
}