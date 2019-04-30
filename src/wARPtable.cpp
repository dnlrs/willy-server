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
        throw net_exception("Cannot get IP Table\n" + wsa_etos(status));
    
}

wARPtable::~wARPtable()
{
    if (ip_table != nullptr)
        FreeMibTable(ip_table);
}

uint64_t
wARPtable::get_mac_from_ip(uint32_t ip_addr)
{
    uint64_t rval = 0;
    for (unsigned long i = 0; i < ip_table->NumEntries; i++) {
        PMIB_IPNET_ROW2 row = &(ip_table->Table[i]);
        
        if (row->Address.Ipv4.sin_addr.s_addr == ip_addr) {
            for (unsigned long j = 0; j < row->PhysicalAddressLength; j++)
                ((uint8_t*) &rval)[j] = row->PhysicalAddress[j];
            break;
        }
    }   
    
    if (rval == 0)
        throw net_exception("Anchor IP not found in ARP table");
    
    return rval;
}
