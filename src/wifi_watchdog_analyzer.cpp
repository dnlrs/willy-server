#include "dealer.h"
#include <utility>

// Link with all the need libs (this works only for the msvc compiler)


//std::string conf_folder("config/");

int main()
{
    std::string configuration_file("config/server.conf");
    dealer dealer;

    // initialize configuration
    dealer.init(configuration_file);

    dealer.start();

    std::this_thread::yield();

    return 0;
}

