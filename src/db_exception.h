#pragma once

#include <string>
using namespace std;

class db_exception : public std::exception
{
public:
    
    db_exception() : errmsg("DB_ERROR: unknown") {}

    db_exception(const char* msg) {
        this->errmsg = "DB_ERROR: " + string(msg);
    }

    virtual const char* what() const throw()
    {
        return (this->errmsg.c_str());
    }
    
private:
    string errmsg;
};

