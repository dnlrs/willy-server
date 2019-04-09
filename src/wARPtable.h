#ifndef WARPTABLE_H_INCLUDED
#define WARPTABLE_H_INCLUDED
#pragma once

#include <WinSock2.h>
#include <ws2def.h>
#include <netioapi.h>

class wARPtable {

public:
    wARPtable(ADDRESS_FAMILY address_family = AF_INET);
    ~wARPtable();

    /* Returns the MAC address of a connected client given its IP address
     *
     * If no matching IP address is found an exception is thrown.
     */
    uint64_t get_mac_from_ip(uint64_t ip_address);

private:
    ADDRESS_FAMILY address_family;
    PMIB_IPNET_TABLE2 ip_table;

};
#endif // !WARPTABLE_H_INCLUDED
