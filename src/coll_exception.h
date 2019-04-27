#ifndef COLL_EXCEPTION_H_INCLUDED
#define COLL_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include <exception>

class coll_exception :
    public std::exception
{
public:
    coll_exception() : 
        errmsg("SHUNTER_EXCEPTION: unknown") 
    {
        debuglog(errmsg);
    }

    coll_exception(std::string msg) : 
        errmsg("SHUNTER_EXCEPTION: " + msg) 
    {
        debuglog(errmsg);
    }

    virtual const char* what() const noexcept 
    { 
        return (this->errmsg.c_str()); 
    }
private:
    std::string errmsg;
};


#endif // !COLL_EXCEPTION_H_INCLUDED
