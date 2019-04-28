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

    int i = 20;
    while (i-- > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}

