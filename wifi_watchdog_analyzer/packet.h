#pragma once
/*
 * packet.h
 *
 *  Created on: 03 mag 2018
 */


#include<cstdint> /* for uint64_t */
#include <string>
#include <sstream>

#define MAC_BYTES 6
#define HASH_LENGHT 32 
#define SSID_LENGHT 32

union mac_t
{
	unsigned char raw_mac[MAC_BYTES];
	UINT64 compacted_mac : 48;
};

typedef struct
{
	std::string hash;
	std::string ssid;
	int rssi;
	mac_t mac_addr;
	UINT8 channel;
	UINT16 off_timestamp; //offset in seconds given by the boards
	time_t timestamp; //in unix time
	UINT32 seq_number;
} PACKET_T;


/* should be inline because when we include this header inside different source files we create multiple definition for that function */
inline std::string mactos(mac_t macaddr)
{
	std::stringstream ss;
	for (int i = 0; i < MAC_BYTES; i++)
	{
		if ( !(macaddr.raw_mac[i] & 0xF0) ) //if the leading digit of a couple is 0 ==> insert it manually
			ss << '0';
		ss << std::hex << (size_t)macaddr.raw_mac[i];
		if(i < MAC_BYTES - 1)
			ss << ':';
	}
	return ss.str();
}

inline std::string timetos(SYSTEMTIME time)
{
	std::string s;
	s = std::to_string(time.wDay) + '/' + std::to_string(time.wMonth) + '/' + std::to_string(time.wYear) + " - " + std::to_string(time.wHour)
		+ ':' + std::to_string(time.wMinute) + ':' + std::to_string(time.wSecond);
	return s;
}


inline void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}


inline void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
	FILETIME ft;

	UnixTimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, pst);
}


