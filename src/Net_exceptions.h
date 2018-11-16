#pragma once

#include <sstream>

using namespace std;

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