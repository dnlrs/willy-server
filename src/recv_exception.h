#ifndef RECV_EXCEPTION_H_INCLUDED
#define RECV_EXCEPTION_H_INCLUDED
#pragma once

#include "net_exception.h"
#include "logger.h"
#include <string>

// this exception it is raised when the connection is closed by the peer
class recv_exception : 
    public net_exception
{
public:
	recv_exception() : 
			errmsg("RECV_EXCEPTION: unknown") {
		debuglog(errmsg);
	}

	recv_exception(const std::string& msg) : 
			errmsg("RECV_EXCEPTION: " + msg) {
		debuglog(errmsg);
	}

	virtual const char* what() const noexcept { return errmsg.c_str(); }

private:
	string errmsg;
};

#endif // !RECV_EXCEPTION_H_INCLUDED