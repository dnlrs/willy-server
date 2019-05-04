#include "tests.h"
#include "dealer.h"
#include "ip_addr.h"
#include "logger.h"
#include "mac_addr.h"
#include "winsock2.h"
#include <cassert>
#include <cstdlib>
#include <string>
#include <utility>

int main()
{
    test_mac_addr();
    test_ip_addr();

    std::string configuration_file("config/server.conf");
    dealer dealer;

    // initialize configuration
    dealer.init(configuration_file);
    dealer.start();

    std::system("pause");

    return 0;
}
