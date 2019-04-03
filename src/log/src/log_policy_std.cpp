#include "log_policy_std.h"
#include <mutex>

// Since there is a single standard output, the mutex must be common
// to all instances of this policy class.
// NOTE: static but non-const data members should be declared outside 
// the class definition because it's considered implementation detail.
std::mutex logging::log_policy_std::write_mutex;

void 
logging::log_policy_std::open_ostream(const std::string& name)
{
    reserved = name;
}

void 
logging::log_policy_std::write(const std::string& msg)
{
    std::unique_lock<std::mutex>
        guard(log_policy_std::write_mutex);

    std::cout << msg << std::endl;
}