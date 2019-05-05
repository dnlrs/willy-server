#ifndef UTL_EXCEPTION_H_INCLUDED
#define UTL_EXCEPTION_H_INCLUDED
#pragma once


#include "logger.h"
#include <exception>
#include <string>

class utl_exception :
    public std::exception
{
public:
    utl_exception() : errmsg("UTILS_EXCEPTION: unknown") { 
        debuglog(errmsg); 
    }
	
    utl_exception(std::string msg) : errmsg("UTILS_EXCEPTION: " + msg) {
        debuglog(errmsg); 
    }

    virtual const char* what() const noexcept { return (this->errmsg.c_str()); }
private:
    std::string errmsg;
};


#endif // !UTL_EXCEPTION_H_INCLUDED