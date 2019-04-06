#ifndef NET_EXCEPTION_H_INCLUDED
#define NET_EXCEPTION_H_INCLUDED
#pragma once

#include <string>
#include "logger.h"

using namespace std;

class net_exception :
    public std::exception
{
public:
    net_exception() : errmsg("NET_ERROR: unknown") { 
        debuglog(errmsg); 
    }
    net_exception(std::string msg) : errmsg("NET_ERROR: " + msg) {
        debuglog(errmsg); 
    }

    virtual const char* what() const throw() { return (this->errmsg.c_str()); }
private:
    std::string errmsg;
};

//invalid socket: when recv() is locked and the dealer fixes the socket, this exception is raised
class Sock_exception : public std::exception
{
public:
	Sock_exception() {
		this->errmsg = string("--[EXCEPTION]-- +++ Sock_exception raised - ");
		this->errcode = WSAGetLastError(); 
	}

	Sock_exception(const string& msg) : Sock_exception() {
		this->errmsg += msg; 
	}

	int geterr() { 
		return this->errcode; 
	}

	virtual string what() throw(){ 	
		errmsg += " (code: " + to_string(this->errcode) + ')';
		return errmsg.c_str(); 
	}

private:
	string errmsg;
	int errcode;
};


// this exception it is raised when the connection is closed by the peer
class Recv_exception
{
public:
	Recv_exception() {
		this->errmsg = string("--[EXCEPTION]-- +++ Recv_exception raised - ");
	}

	Recv_exception(const string &msg) : Recv_exception() {
		this->errmsg += msg;
	}

	virtual const char* what() throw() {
		return this->errmsg.c_str(); 
	}

private:
	string errmsg;

};

#endif // !NET_EXCEPTION_H_INCLUDED
