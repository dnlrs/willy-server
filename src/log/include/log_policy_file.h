#ifndef LOG_POLICY_FILE_H_INCLUDED
#define LOG_POLICY_FILE_H_INCLUDED

#include "log_policy_interface.h"
#include "log_exception.hpp"
#include <memory>
#include <fstream>
#include <mutex>
#include <string>
#include <map>

namespace logging
{
    // Logs to file. 
    // - Thread safe
    // - Multiple loggers can be opened towards the same file
    // - NB: the first time opening a file its content is deleted
    class log_policy_file : 
        public log_policy_interface
    {
    public:
        log_policy_file() : 
            out_stream(std::make_unique<std::ofstream>()) {}
        
        ~log_policy_file() { close_ostream(); }

        void open_ostream(const std::string& file_out);

        void close_ostream();

        void write(const std::string& msg);

    private:
        std::string 
            file_name;
        std::unique_ptr<std::ofstream> 
            out_stream = nullptr;

        static std::mutex 
            mutex_map_access;
        static std::map<std::string, std::unique_ptr<std::mutex>> 
            mutex_map;
    };
};

#endif // LOG_POLICY_FILE_HPP_INCLUDED

