#ifndef WWSADATA_H_INCLUDED
#define WWSADATA_H_INCLUDED

#include "logger.h"
#include "net_exception.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class wwsadata
{
public:
    wwsadata() 
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        
        if (WSAStartup(wVersionRequested, &data) != 0)
            throw net_exception("Winsock initialization failed");
            
        debuglog("wwsadata ctor completed");
    }

    ~wwsadata() {
        WSACleanup();
        debuglog("wwsadata dtor completed");
    }

private:
    WSADATA data;
};

#endif // !WWSADATA_H_INCLUDED
