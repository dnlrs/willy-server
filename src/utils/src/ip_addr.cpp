#include "ip_addr.h"

ip_addr::ip_addr(
    std::string str_ip, 
    bool is_host_byte_order = true) :
        is_hbo(is_host_byte_order)
{
    int rc = inet_pton(AF_INET, str_ip.c_str(), &addr);

    if (rc == 0)
        throw utl_exception(
            "ip_addr ctor: string is not a valid IPv4 dotted-decimal string");
    if (rc == -1)
        throw utl_exception(
            "ip_addr ctor: ctor failed\n" + wsa_etos(WSAGetLastError()));
}

ip_addr 
ip_addr::hton()
{
    if (is_hbo == false)
        return ip_addr(*this);

    return ip_addr(::htonl(addr), false);
}

ip_addr 
ip_addr::ntoh()
{
    if (is_hbo == true)
        return ip_addr(*this);
    return ip_addr(::ntohl(addr), true);
}

std::string 
ip_addr::str()
{
    if (is_hbo == false)
        return ntoh().str();

    char str[INET_ADDRSTRLEN + 1]; // 192.168.255.255[:port]
    memset(&str[0], 0, INET_ADDRSTRLEN + 1);

    const char* rc = inet_ntop(AF_INET, &addr, &str[0], INET_ADDRSTRLEN + 1);
    if (rc == nullptr) {
        debuglog("ip_addr::str(): failed to convert ip address\n",
            wsa_etos(WSAGetLastError()));
        return std::string("");
    }

    str[INET_ADDRSTRLEN] = '\0';
    return std::string(str);
}