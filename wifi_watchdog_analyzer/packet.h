#pragma once
/*
 * packet.h
 *
 *  Created on: 03 mag 2018
 */


#include<cstdint> /* for uint64_t */
#include <string>
#include <sstream>

#define MAC_LENGTH 6
#define MD5_HASH_LENGTH 32
#define MAX_SSID_LENGTH 32

union mac_t
{
	unsigned char raw_mac[MAC_LENGTH];
	UINT64 compacted_mac : 48;
};

typedef struct
{
	int channel;
	int rssi;
	int sequence_ctrl;
	int timestamp;
	mac_t mac_addr;
	std::string ssid;
	std::string hash;
} PACKET_T;


/* should be inline because when we include this header inside different source files we create multiple definition for that function */
inline std::string mactos(mac_t macaddr)
{
	std::stringstream ss;
	for (int i = 0; i < MAC_LENGTH; i++)
	{
		if ( !(macaddr.raw_mac[i] & 0xF0) ) //if the leading digit of a couple is 0 ==> insert it manually
			ss << '0';
		ss << std::hex << (size_t)macaddr.raw_mac[i];
		if(i < MAC_LENGTH - 1)
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


PACKET_T inline deserialize(char *buf) {
	int channel;
	int rssi;
	int sequence_ctrl;
	int timestamp;
	int ssid_length;
	char mac_addr[MAC_LENGTH];
	char ssid[MAX_SSID_LENGTH + 1];
	char hash[MD5_HASH_LENGTH + 1];

	int *buf_int = (int *)buf;
	channel = ntohl(*buf_int); //CHANNEL
	buf_int++;
	rssi = ntohl(*buf_int); //RSSI
	buf_int++;
	sequence_ctrl = ntohl(*buf_int); //SEQUENCE CTRL
	buf_int++;
	timestamp = ntohl(*buf_int); //TIMESTAMP
	buf_int++;
	ssid_length = ntohl(*buf_int); //SSID LENGTH
	buf_int++;

	char *buf_c = (char *)buf_int;

	for (int i = 0; i < MAC_LENGTH; i++) {
		mac_addr[i] = *buf_c;
		buf_c++;
	}

	for (int i = 0; i < MD5_HASH_LENGTH; i++) {
		hash[i] = *buf_c;
		buf_c++;
	}
	hash[MD5_HASH_LENGTH] = '\0';

	for (int i = 0; i < ssid_length; i++) {
		ssid[i] = *buf_c;
		buf_c++;
	}
	ssid[ssid_length] = '\0';

	printf("CH=%d, RSSI=%02d, MAC=%02x:%02x:%02x:%02x:%02x:%02x, SSID=%s, SEQ=%d, TIMESTAMP: %d, HASH=%s\n",
		channel,
		rssi,
		(uint8_t)mac_addr[0], (uint8_t)mac_addr[1], (uint8_t)mac_addr[2],
		(uint8_t)mac_addr[3], (uint8_t)mac_addr[4], (uint8_t)mac_addr[5],
		ssid,
		sequence_ctrl,
		timestamp,
		hash
	);

	PACKET_T pack;
	pack.channel = channel;
	pack.rssi = rssi;
	for(int i = 0; i < MAC_LENGTH; i ++)
		pack.mac_addr.raw_mac[i] = mac_addr[i];
	pack.ssid = std::string(ssid);
	pack.sequence_ctrl = sequence_ctrl;
	pack.timestamp = timestamp;
	pack.hash = hash;

	return pack;
}