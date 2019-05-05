#ifndef DB_EXCEPTION_H_INCLUDED
#define DB_EXCEPTION_H_INCLUDED
#pragma once

#include "logger.h"
#include <string>

#define DBE "DB_EXCEPTION: "

namespace db
{
    class db_exception:
        public std::exception
    {
    public:

        enum type
        {
            error,              // unrecovable error
            constraint_error    // trying to insert a duplicate tuple
        };

        db_exception() :
            errmsg(DBE "unknown"),
            errtype(error)
        {
            debuglog(errmsg);
        }

        db_exception(const char* msg, type etype = error) :
            errmsg(DBE + std::string(msg)),
            errtype(etype)
        {
            debuglog(errmsg);
        }

        virtual const char* what() const throw()
        {
            return (this->errmsg.c_str());
        }

        virtual const type why()
        {
            return this->errtype;
        }

    private:
        std::string errmsg;
        type        errtype;
    };
}

#endif // !DB_EXCEPTION_H_INCLUDED
