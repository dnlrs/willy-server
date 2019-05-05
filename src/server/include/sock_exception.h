#ifndef SOCK_EXCEPTION_H_INCLUDED
#define SOCK_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include "net_exception.h"
#include "utils.h"
#include <string>
#include <WinSock2.h>

#define SOCKE "SOCK_EXCEPTION: "

/*
 * This is an exception regardin sockets, invalid sockets, 
 * recv / write failures that indicate dead connections.
 * Usually it's not a fatal error, but it may be.
 */
class sock_exception
{
public:
    sock_exception() :
        errmsg(SOCKE "unknown") {
        debuglog(errmsg);
    }

    sock_exception(const std::string& msg, SOCKET in_sock) :
        errmsg(SOCKE + msg),
        sock(in_sock) {
        debuglog(errmsg);
    }

    sock_exception(const std::string& msg, int wsa_error, SOCKET in_sock) :
        errmsg(SOCKE + msg + wsa_etos(wsa_error)),
        errcode(wsa_error),
        sock(in_sock) {
        debuglog(errmsg);
    }

    virtual const char* what() const noexcept { return errmsg.c_str(); }

    int geterr() { return this->errcode; }
    SOCKET get_socket() { return sock; }

private:
    std::string errmsg;
    int errcode = 0;
    SOCKET sock = INVALID_SOCKET;
};

#endif // !SOCK_EXCEPTION_H_INCLUDED
