#ifndef SOCK_EXCEPTION_H_INCLUDED
#define SOCK_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include "net_exception.h"
#include "utils.h"
#include <string>
#include <WinSock2.h>

class sock_exception : 
    public net_exception
{
public:
	sock_exception() : 
			errmsg("SOCK_EXCEPTION: unknown") {
		debuglog(errmsg);
	}

	sock_exception(const std::string& msg, SOCKET in_sock) : 
			errmsg("SOCK_EXCEPTION: " + msg),
            sock(in_sock) {
		debuglog(errmsg);
	}
	
	sock_exception(const std::string& msg, int wsa_error, SOCKET in_sock) :
			errmsg("SOCK_EXCEPTION: " + msg + wsa_etos(wsa_error)),
			errcode(wsa_error),
            sock(in_sock) {
		debuglog(errmsg);
	}

	virtual const char* what() const noexcept { return errmsg.c_str(); }

	int geterr() { return this->errcode; }
    SOCKET get_socket() { return sock; }

private:
	string errmsg;
	int errcode = 0;
    SOCKET sock = INVALID_SOCKET;
};

#endif // !SOCK_EXCEPTION_H_INCLUDED