#ifndef MAC_ADDR_H_INCLUDED
#define MAC_ADDR_H_INCLUDED
#pragma once

#include "utl_exception.h"
#include <cctype>
#include <cstdint>
#include <string>

constexpr int mac_length = 6;

/* MAC address wrapper */
class mac_addr {

public:
    mac_addr();
    mac_addr(uint8_t* addr_in, bool is_host_byte_order = false);
    mac_addr(std::string str_mac, bool is_host_byte_order = true);
    mac_addr(const mac_addr& other); // copy ctor

    mac_addr operator=(std::string str_mac) { return mac_addr(str_mac); }
    uint8_t& operator[](int index);

    bool operator==(uint64_t uint64_mac) const  { return uint64() == uint64_mac;     }
    bool operator==(const mac_addr& other) const      { return uint64() == other.uint64(); }
    bool operator==(std::string str_mac) const  { return uint64() == mac_addr(str_mac).uint64(); }
    bool operator<(uint64_t uint64_mac) const   { return uint64() < uint64_mac;      }
    bool operator<(const mac_addr& other) const       { return uint64() < other.uint64();  }
    bool operator<(std::string str_mac) const   { return uint64() < mac_addr(str_mac).uint64(); }

    void set_host_byte_order()    { is_hbo = true; }
    void set_network_byte_order() { is_hbo = false; };

    bool is_valid();
    void clear();

    mac_addr ntoh();
    mac_addr hton();

    std::string str() const;
    uint64_t    uint64() const;

private:

    void change_byte_order();

    /* Checks if a mac in string format is valid */
    int mac_is_valid(const char* mac);

    static uint8_t hex_to_uint8(char hex_char);

private:
    uint8_t addr[mac_length];
    bool is_hbo; // is Host Byte Order?

};

#endif // !MAC_ADDR_H_INCLUDED