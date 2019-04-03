#ifndef LOG_POLICY_STD_INCLUDED
#define LOG_POLICY_STD_INCLUDED

#include "log_policy_interface.h"
#include <string>
#include <iostream>
#include <mutex>


namespace logging
{
    // Logs to standard output.
    class log_policy_std : 
        public log_policy_interface
    {
    public:
        void open_ostream(const std::string& name = "");
        void close_ostream() {}
        void write(const std::string& msg);

    private:
        std::string reserved;

        static std::mutex write_mutex;
    };
}

#endif // LOG_POLICY_STD_INCLUDED
