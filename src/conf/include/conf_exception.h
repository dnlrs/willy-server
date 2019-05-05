#ifndef CONF_EXCEPTION_H_INCLUDED
#define CONF_EXCEPTION_H_INCLUDED

#include <string>
#include <exception>

#define CONF_UNKNOWN			0 // unknown/unspecified error
#define CONF_FILE_NOT_FOUND 	1 // file not found 
#define CONF_BAD_SYNTAX_XML     2 // bad syntax while parsing file
#define CONF_INVALID_XML        3 // bax xml element names or attributes...
#define CONF_NO_ANCHORS         4 // no anchors found
#define CONF_FILE_NOT_CREATED   5 // could not create specified file
#define CONF_MARSHALL_ERROR     6 // could not write xml on file
#define CONF_BAD_ALLOC          7 // could not allocate xml string

class conf_exception : public std::exception
{
public:
	conf_exception() : 
		errmsg("CONF_ERROR: unknown"),
        errcode(0) {}

    conf_exception(std::string msg) : 
        errmsg("CONF_ERROR: " + msg), 
        errcode(0) {}
	
    conf_exception(std::string msg, int errcode) : 
        errmsg("CONF_ERROR: " + msg), 
        errcode(errcode) {}

	virtual const char* what() const throw() { return (this->errmsg.c_str()); }
	virtual const int   which() const throw() { return this->errcode; }
private:
	int errcode;
	std::string errmsg;
};

#endif // !CONF_EXCEPTION_H_INCLUDED
