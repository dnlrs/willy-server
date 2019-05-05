#ifndef RECV_EXCEPTION_H_INCLUDED
#define RECV_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include "net_exception.h"
#include <string>

#define RECVE "RECV_EXCEPTION: "

/* Currently not used.
 * This exception is raised when the connection is closed
 * by the peer
 */
class recv_exception
{
public:
    recv_exception() : errmsg(RECVE "unknown") {
        debuglog(errmsg);
    }

    recv_exception(const std::string& msg) : errmsg(RECVE + msg) {
        debuglog(errmsg);
    }

    virtual const char* what() const noexcept { return errmsg.c_str(); }

private:
    std::string errmsg;
};

#endif // !RECV_EXCEPTION_H_INCLUDED
