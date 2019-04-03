#ifndef LOG_POLICY_INTERFACE_H_INCLUDED
#define LOG_POLICY_INTERFACE_H_INCLUDED

#include <string>

namespace logging
{
    class log_policy_interface
    {
    public:
        virtual void open_ostream(const std::string& name) = 0;
        virtual void close_ostream() = 0;
        virtual void write(const std::string& msg) = 0;
    };
}
#endif // !LOG_POLICY_H_INCLUDED