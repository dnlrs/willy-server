#ifndef MAC_ADDR_H_INCLUDED
#define MAC_ADDR_H_INCLUDED
#pragma once

#include "utl_exception.h"
#include <cctype>
#include <cstdint>
#include <string>

/* MAC address wrapper
 *
 * The MAC addres is stored in canonical form (network byte order).
 * And network byte order = big endian.
 *
 * note sul byte order:
 *
 * string: aa:bb:cc:dd:ee:ff
 * int:    0x0000aabbccddeeff
 *
 * big endian (network byte order):
 *     |00|00|aa|bb|cc|dd|ee|ff|
 *      -  -  0  1  2  3  4  5
 *
 * little endian (host byte order):
 *     |ff|ee|dd|cc|bb|aa|00|00|
 *      0  1  2  3  4  5  -  -
 * */
class mac_addr {
public:
    mac_addr();
    mac_addr(const std::string str_mac);
    mac_addr(const mac_addr& other); // copy ctor

    mac_addr& operator=(const std::string& str_mac);
    mac_addr& operator=(uint64_t int_mac);

    uint8_t& operator[](int index);

    // all comparisons are done in canonical form
    bool operator==(const mac_addr& other) const;
    bool operator==(uint64_t uint64_mac) const;
    bool operator==(const std::string str_mac) const;
    bool operator<(const mac_addr& other) const;
    bool operator<(uint64_t uint64_mac) const;
    bool operator<(const std::string str_mac) const;

    bool is_valid() const;
    void clear();

    std::string str() const; // conversion to string
    uint64_t    get() const; // convertion to unsigned long long

public:
    static const int mac_length = 6;

private:
    void parse_mac_string(std::string mac_str);
    int  is_valid_mac_string(const char* mac);

    static uint8_t convert_hex_to_int(char hex_char);

private:
    uint8_t addr[mac_length];
};

#endif // !MAC_ADDR_H_INCLUDED