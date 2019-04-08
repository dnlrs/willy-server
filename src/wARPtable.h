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

private:
    ADDRESS_FAMILY address_family;
    PMIB_IPNET_TABLE2 ip_table;

};
#endif // !WARPTABLE_H_INCLUDED
