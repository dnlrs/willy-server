#ifndef SOCK_EXCEPTION_H_INCLUDED
#define SOCK_EXCEPTION_H_INCLUDED
#pragma once

#include "net_exception.h"
#include "logger.h"
#include "utils.h"
#include <string>

class sock_exception : 
    public net_exception
{
public:
	sock_exception() : 
			errmsg("SOCK_EXCEPTION: unknown") {
		debuglog(errmsg);
	}

	sock_exception(const std::string& msg) : 
			errmsg("SOCK_EXCEPTION: " + msg) {
		debuglog(errmsg);
	}
	
	sock_exception(const std::string& msg, int wsa_error) :
			errmsg("SOCK_EXCEPTION: " + msg + wsa_etos(wsa_error)),
			errcode(wsa_error) {
		debuglog(errmsg);
	}

	virtual const char* what() const noexcept { return errmsg.c_str(); }

	int geterr() { return this->errcode; }
	
private:
	string errmsg;
	int errcode;
};

#endif // !SOCK_EXCEPTION_H_INCLUDED