#pragma once

#include <sstream>
#include <thread>

using namespace std;

class Conf_exception : public std::exception
{
public:
	Conf_exception() { 
		this->errmsg =  "Configuration file error - "; 
	}

	Conf_exception(const char* msg) {
		this->errmsg += msg;
	}

	virtual const char* what() const throw()
	{
		return (this->errmsg.c_str());
	}
private:
	string errmsg;
};
