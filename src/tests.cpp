#include "tests.h"

#include "ip_addr.h"
#include "mac_addr.h"
#include "winsock2.h"
#include <cassert>
#include <cstdint>
#include <string>


void test_mac_addr()
{
    std::string mac_str("30:AE:A4:75:1F:20");
    uint64_t    mac_int_n = 0x000030aea4751f20;
    uint64_t    mac_int_h = ntohll(mac_int_n);
    uint64_t    mac_int2_n = 0x000030aea4751f21;

    mac_addr mac1(mac_str); // ctor from string
    debuglog("mac1", mac1.str());
    mac_addr mac2(mac1); // ctor from mac_addr
    debuglog("mac2", mac2.str());
    mac_addr mac3;
    mac3 = mac_str; // copy assign from string
    debuglog("mac3", mac3.str());
    mac_addr mac4;
    mac4 = mac_int2_n; // copy assign from int
    debuglog("mac4", mac4.str());

    debuglog("hbo mac_int", std::to_string(mac_int_h));
    debuglog("nbo mac_int", std::to_string(mac_int_n));
    debuglog("mac1 int", std::to_string(mac1.get()));
    debuglog("mac3 int", std::to_string(mac3.get()));
    debuglog("mac4 int", std::to_string(mac3.get()));

    assert(mac1 == mac2);
    assert(mac2 == mac3);

    assert(mac3 == mac_int_n);
    assert(ntohll(mac3.get()) == mac_int_h);
    assert(mac1 < mac4);

    assert(ntohll(mac1.get()) == mac_int_h);

    debuglog("original mac is", mac1.str());
    debuglog("network byte order is", mac1.get());
    debuglog("host byte order is", ntohll(mac1.get()));
}

void test_ip_addr()
{
    std::string ip_str("192.168.1.12");

    ip_addr ip1(ip_str);
    debuglog("ip1", ip1.str());
    debuglog("ip1 int network byte order", std::to_string(ip1.get()));
    debuglog("ip1 int host byte order", std::to_string(ntohl(ip1.get())));

    ip_addr ip2(ip1);
    assert(ip2 == ip1);
    assert(ip2 == 201435328);
}