#include "net_exception.h"
#include "utils.h"
#include "wARPtable.h"

wARPtable::wARPtable(ADDRESS_FAMILY address_family) :
    address_family(AF_INET),
    ip_table(nullptr)
{
    unsigned long status;

    status = GetIpNetTable2(address_family, &ip_table);
    if (status != NO_ERROR)
        throw net_exception("wARPtable::ctor: "
            "cannot get IP Table\n" + wsa_etos(status));
}

wARPtable::~wARPtable()
{
    if (ip_table != nullptr)
        FreeMibTable(ip_table);
}

mac_addr
wARPtable::get_mac_from_ip(ip_addr ip)
/*
 * Both IP and MAC addresses in the arp table are in
 * network byte order (big endian).
 */
{
    mac_addr rval;
    for (unsigned long i = 0; i < ip_table->NumEntries; i++) {
        PMIB_IPNET_ROW2 row = &(ip_table->Table[i]);

        if (ip == row->Address.Ipv4.sin_addr.s_addr) {
            for (unsigned long j = 0; j < row->PhysicalAddressLength; j++)
                rval[j] = row->PhysicalAddress[j];
            break;
        }
    }

    if (rval.is_valid() == false)
        throw net_exception("wARPtable::get_mac_from_ip: "
            "anchor IP not found in ARP table");

    return rval;
}
