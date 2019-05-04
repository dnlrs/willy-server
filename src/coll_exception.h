#ifndef COLL_EXCEPTION_H_INCLUDED
#define COLL_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include <exception>

#define COLLE "COLL_EXCEPTION: "

class coll_exception:
    public std::exception
{
public:
    coll_exception() :
        errmsg(COLLE "unknown")
    {
        debuglog(errmsg);
    }

    coll_exception(std::string msg) :
        errmsg(COLLE + msg)
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
