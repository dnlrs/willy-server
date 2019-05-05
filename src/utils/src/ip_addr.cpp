#include "ip_addr.h"

ip_addr::ip_addr(
    const std::string str_ip)
{
    int rc = inet_pton(AF_INET, str_ip.c_str(), &addr);

    if (rc == 0)
        throw utl_exception(
            "ip_addr ctor: string is not a valid IPv4 dotted-decimal string");
    if (rc == -1)
        throw utl_exception(
            "ip_addr ctor: ctor failed\n" + wsa_etos(WSAGetLastError()));
}

ip_addr& ip_addr::operator=(const uint32_t ip_addr)
{
    addr = ip_addr;
    return *this;
}

std::string
ip_addr::str() const
{
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