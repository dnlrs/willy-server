#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include "log_policies.h"
#include "log_exception.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

// TODO: move default file in app configuration
#define DEFAULT_LOG_FILE "glog_willy.log"

namespace logging
{
    // BIG NOTE: templated classes cannot have function
    // declarations and function definitions in 2 
    // different files

    enum severity_type {
        debug,
        info,
        warning,
        error
    };

    template<typename log_policy>
    class logger
    {

    public:
        logger(const std::string& name)
        {
            //policy = std::unique_ptr<log_policy>(
            //            std::make_unique<log_policy>());

            policy = std::make_unique<log_policy>();

            if (policy == nullptr)
                throw log_exception("logger could not instantiate policy");

            policy->open_ostream(name);
        }

        // debug level log
        // Note: this level is available only 
        // during development
        template<typename ... Args>
        void debug(Args ... args)
        {
#ifndef NDEBUG
            print<logging::severity_type::debug>(args...);
#else
            ; // do not print anything
#endif // !NDEBUG
        }

        // info level log
        template<typename ... Args>
        void info(Args ... args) 
        {
            print<logging::severity_type::info>(args...);
        }

        // warning level log
        template<typename ... Args>
        void warning(Args ... args) 
        {
            print<logging::severity_type::warning>(args...);
        }

        // error level log
        template<typename ... Args>
        void error(Args ... args) 
        {
            print<logging::severity_type::error>(args...);
        }


    private:
        template<severity_type severity, typename ... Args>
        void print(Args ... args)
        {
            std::stringstream log_stream("");
            log_stream << get_time();

            switch (severity)
            {
            case severity_type::debug:
                log_stream << "DEBUG  ";
                break;
            case severity_type::info:
                log_stream << "INFO   ";
                break;
            case severity_type::warning:
                log_stream << "WARNING";
                break;
            case severity_type::error:
                log_stream << "ERROR  ";
                break;
            default:
                log_stream << "UNKNOWN";
                break;
            }
            print_impl(log_stream, args...);
        }

        template<typename First, typename ... Rest>
        void print_impl(
            std::stringstream& log_stream, 
            First arg, Rest ... args)
        {
            log_stream << arg << " ";
            print_impl(log_stream, args...);
        }

        void print_impl(std::stringstream& log_stream)
        {
            policy->write(log_stream.str());
            log_stream.str("");
        }

        std::string get_time()
        {
            time_t instant = time(nullptr);
            if (instant == (time_t)-1)
                throw log_exception("logger could not get current time");

            struct tm instant_loc;
            if (localtime_s(&instant_loc, &instant) != 0)
                throw log_exception("logger could not convert time to local time");

            char instant_str[32];
            if (strftime(instant_str, 64, "%Y-%m-%d %H:%M:%S ", &instant_loc) == 0)
                throw log_exception("logger could not convert local tmie to string");
            instant_str[21] = '\0';

            return std::string(instant_str);
        }

    private:
        std::unique_ptr<log_policy> policy = nullptr;
    };
};


#ifndef NDEBUG

// a global logger active only while debugging
static logging::logger<logging::log_policy_std> glog(DEFAULT_LOG_FILE);

#define debuglog glog.debug

#else

#define debuglog(...)

#endif // !NDEBUG

#endif // LOGGER_H_INCLUDED