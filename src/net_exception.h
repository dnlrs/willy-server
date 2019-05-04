#ifndef NET_EXCEPTION_H_INCLUDED
#define NET_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include <exception>
#include <string>

#define NETE "NET_EXCEPTION: "

/* 
 * This is quite a fatal error and whever it is raised, 
 * generally the server is immediately shut down.
 */
class net_exception:
    public std::exception
{
public:
    net_exception() : errmsg(NETE "unknown") {
        debuglog(errmsg);
    }

    net_exception(std::string msg) : errmsg(NETE + msg) {
        debuglog(errmsg);
    }

    virtual const char* what() const noexcept { return (this->errmsg.c_str()); }
private:
    std::string errmsg;
};

#endif // !NET_EXCEPTION_H_INCLUDED
