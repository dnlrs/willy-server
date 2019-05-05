#ifndef IPS_EXCEPTION_H_INCLUDED
#define IPS_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include <exception>
#include <string>

#define IPSE "IPS_EXCEPTION: "

class ips_exception:
    public std::exception
{
public:
    ips_exception() : errmsg(IPSE "unknown") {
        debuglog(errmsg);
    }

    ips_exception(std::string msg) : errmsg(IPSE + msg) {
        debuglog(errmsg);
    }

    virtual const char* what() const noexcept { return (this->errmsg.c_str()); }
private:
    std::string errmsg;
};

#endif // !IPS_EXCEPTION_H_INCLUDED