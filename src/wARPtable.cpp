#include "wARPtable.h"
#include "utils.h"
#include "net_exception.h"

wARPtable::wARPtable(ADDRESS_FAMILY address_family) :
    address_family(AF_INET),
    ip_table(nullptr)
{
    unsigned long status;

    status = GetIpNetTable2(address_family, &ip_table);
    if (status != NO_ERROR) {
        throw net_exception("Cannot get IP Table\n" + wsa_etos(status));
    }
}

wARPtable::~wARPtable()
{
    if (ip_table != nullptr)
        FreeMibTable(ip_table);
}

