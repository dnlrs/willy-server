#ifndef IP_ADDR_H_INCLUDED
#define IP_ADDR_H_INCLUDED
#pragma once

#include "logger.h"
#include "utl_exception.h"
#include "utils.h"
#include "ws2tcpip.h"

class ip_addr {

public:
    ip_addr(uint32_t ip = 0, bool is_host_byte_order = false) : 
        addr(ip), is_hbo(is_host_byte_order) {}

    ip_addr(std::string str_ip, bool is_host_byte_order = true);

    ip_addr(const ip_addr& other) : 
        addr(other.addr), is_hbo(other.is_hbo) {} // copy ctor

    bool is_valid() { return (addr != 0); }
    void clear()    { addr = 0; is_hbo = false; }

    bool operator==(const uint32_t other) { return addr == other; }
    bool operator==(const ip_addr other)  { return addr == other.addr; }
    bool operator<(const ip_addr other)   { return addr < other.addr; }

    void set_host_byte_order()    { is_hbo = true; }
    void set_network_byte_order() { is_hbo = false; }
    
    ip_addr hton() const;
    ip_addr ntoh() const;

    std::string str() const;


private:
    uint32_t addr;
    bool is_hbo;

};

#endif // !IP_ADDR_H_INCLUDED