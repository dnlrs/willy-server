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
    net_exception() : errmsg("NET_EXCEPTION: unknown") { 
        debuglog(errmsg); 
    }
	
    net_exception(std::string msg) : errmsg("NET_EXCEPTION: " + msg) {
        debuglog(errmsg); 
    }

    virtual const char* what() const noexcept { return (this->errmsg.c_str()); }
private:
    std::string errmsg;
};




#endif // !NET_EXCEPTION_H_INCLUDED
