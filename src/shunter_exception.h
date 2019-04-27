#ifndef SHUNTER_EXCEPTION_H_INCLUDED
#define SHUNTER_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"

class shunter_exception :
    public std::exception
{
public:
    shunter_exception() : 
        errmsg("SHUNTER_EXCEPTION: unknown") 
    {
        debuglog(errmsg);
    }

    shunter_exception(std::string msg) : 
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


#endif // !SHUNTER_EXCEPTION_H_INCLUDED
