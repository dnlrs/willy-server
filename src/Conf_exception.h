#pragma once


class Conf_exception : public std::exception
{
public:
	Conf_exception() { 
		this->errmsg =  "--[EXCEPTION]-- +++ Configuration file error - "; 
	}

	Conf_exception(const char* msg) : Conf_exception() {
		this->errmsg += msg;
	}

	virtual const char* what() const throw(){
		return errmsg.c_str();
	}
private:
	std::string errmsg;
};
