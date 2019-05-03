#ifndef IP_ADDR_H_INCLUDED
#define IP_ADDR_H_INCLUDED
#pragma once

#include "logger.h"
#include "utl_exception.h"
#include "utils.h"
#include "ws2tcpip.h"

/* IP address wrapper
 *
 * The IP address is stored as unsigned long (uint32_t)
 * in network byte order.
 *
 * Note: the internet standard text representation is in
 * host byte order. i.e. "192.168.0.1" is host byte order.
 */
class ip_addr {
public:
    ip_addr(uint32_t ip = 0) : addr(ip) {}
    ip_addr(const std::string str_ip);
    ip_addr(const ip_addr& other) : addr(other.addr) {} // copy ctor

    ip_addr& operator=(const uint32_t ip_addr);

    bool is_valid() const { return (addr != 0); }
    void clear() { addr = 0; }

    bool operator==(const ip_addr& other) const { return this->addr == other.addr; }
    bool operator==(const uint32_t other) const { return this->addr == other; }
    bool operator<(const ip_addr& other) const { return this->addr < other.addr; }

    std::string str() const;
    uint32_t    get() { return addr; }
    uint32_t&   data() { return addr; }

private:
    uint32_t addr;
};

#endif // !IP_ADDR_H_INCLUDED