#pragma once

#include <string>

namespace db {
    
    

    class db_exception : public std::exception
    {
    public:

        enum type {
            error,              // unrecovable error
            constraint_error    // trying to insert a duplicate tuple
        };
    
        db_exception() : 
            errmsg("DB_ERROR: unknown"), 
            errtype(error) {}

        db_exception(const char* msg, type etype = error) {
            this->errmsg = "DB_ERROR: " + std::string(msg);
            errtype      = etype;
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


