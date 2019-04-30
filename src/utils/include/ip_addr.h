#ifndef IP_ADDR_H_INCLUDED
#define IP_ADDR_H_INCLUDED
#pragma once

#include "logger.h"
#include "utl_exception.h"
#include "utils.h"
#include "ws2tcpip.h"

class ip_addr {

    ip_addr(uint32_t ip = 0, bool is_host_byte_order = false) : 
        addr(ip), is_hbo(is_host_byte_order) {}

    ip_addr(std::string str_ip, bool is_host_byte_order = true);

    ip_addr(const ip_addr& other) : 
        addr(other.addr), is_hbo(other.is_hbo) {}


    bool is_valid() { return (addr != 0); }

    void set_host_byte_order()    { is_hbo = true; }
    void set_network_byte_order() { is_hbo = false; }
    
    ip_addr hton();
    ip_addr ntoh();

    std::string str();

private:
    uint32_t addr;
    bool is_hbo;

};

#endif // !IP_ADDR_H_INCLUDED