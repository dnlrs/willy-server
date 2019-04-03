#ifndef LOG_EXCEPTION_HPP_INCLUDED
#define LOG_EXCEPTION_HPP_INCLUDED

#include <string>
#include <exception>


class log_exception : public std::exception
{
public:
    log_exception() : 
        errmsg("LOG_ERROR: unknown") {}

    log_exception(std::string msg) : 
        errmsg("LOG_ERROR" + msg) {}

    virtual const char* what() const throw() { return errmsg.c_str(); }

private:
    std::string errmsg;

};

#endif // !LOG_EXCEPTION_HPP_INCLUDED