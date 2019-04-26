#ifndef SOCKET_UTILS_H_INCLUDED
#define SOCKET_UTILS_H_INCLUDED
#pragma once

#include <WinSock2.h>

// no activity timeout (ms) until the first keepalive packet is sent (1 min)
constexpr unsigned long int default_keepalive_time_ms = 1000 * 60;
// inverval between keepalive packets if no response received (1 sec)
constexpr unsigned long int default_keepalive_interval_ms = 1000;

// local buffer size whean reading from socket
constexpr long int default_buffer_size = 1024;

// sleep time if the nonblocking socket would block in ms
constexpr long int default_wouldblock_sleep    = 100;
// max attempts waiting for a non-blocking socket
constexpr long int default_wouldblock_attempts = 5;


/* sets the SO_KEEPALIVE option */
void set_keepalive_option(
    const SOCKET anchor_socket);

/* set socket non-blocking */
void set_non_blocking_socket(
    const SOCKET socket_in);

/* Reads a message size and a message from the socket
 * 
 * First reads the numbers of bytes to be read (as uint32_t) then reads 
 * the actual bytes.
 * If an error occurs throws an exception.
 * 
 * Returns the length of the read message.
 */
uint32_t
read_sized_message(
    std::vector<uint8_t>& rval,
    const SOCKET raw_socket);

/* Reads the specified number of bytes from the socket
 * 
 * In case the socket is nonblocking and there is no data to be read
 * the function will sleep for $(default_wouldblock_sleep) and will
 * make $(default_wouldblock_attempts).
 * If no attempt is successfull throws an exception.  
 * 
 * Returns the length of the read message.
 */
uint32_t
read_n(
    uint8_t* const dst_buffer,
    const uint32_t msg_length,
    const SOCKET raw_socket);

#endif // !SOCKET_UTILS_H_INCLUDED
