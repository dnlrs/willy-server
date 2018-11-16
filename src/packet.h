#pragma once
/*
 * packet.h
 *
 *  Created on: 03 mag 2018
 */


#include <cstdint> /* for uint64_t */
#include <sstream>
#include <Windows.h>
#include <time.h>
#include <vector>


#define MAC_LENGTH 6
#define MD5_HASH_LENGTH 32
#define MAX_SSID_LENGTH 32
#define BOARD_PACK_DIM 84



union mac_t
{
	unsigned char raw_mac[MAC_LENGTH];
	UINT64 compacted_mac : 48;
};

typedef struct
{
	uint32_t channel;
	int32_t rssi;
	uint32_t sequence_ctrl;
	uint32_t timestamp;
	uint32_t ssid_lenght;
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

inline std::string timetos(struct tm time/*SYSTEMTIME time*/)
{
	/*std::string s;
	s = std::to_string(time.wDay) + '/' + std::to_string(time.wMonth) + '/' + std::to_string(time.wYear) + " - " + std::to_string(time.wHour)
		+ ':' + std::to_string(time.wMinute) + ':' + std::to_string(time.wSecond);
	return s;*/
	std::string s;
	std::vector<std::string> days(7);
	std::vector<std::string> months(12);
	days[0] = "Sunday"; days[1] = "Monday"; days[2] = "Tuesday"; days[3] = "Wednesday"; days[4] = "Thursday"; days[5] = "Friday"; days[6] = "Saturday";
	months[0] = "January"; months[1] = "February"; months[2] = "March"; months[3] = "April"; months[4] = "May"; months[5] = "June"; months[6] = "July";
	months[7] = "August"; months[8] = "September"; months[9] = "October"; months[10] = "November"; months[11] = "December";
	s += days[time.tm_wday] + ", " + months[time.tm_mon] + " " + std::to_string(time.tm_mday) + ", " + std::to_string(1900+time.tm_year)
		+ " " + std::to_string(time.tm_hour) + ":" + std::to_string(time.tm_min) + ":" + std::to_string(time.tm_sec);
	return s;

}

inline struct tm epochTotm(const time_t rawtime)
{
	struct tm ptm;
	gmtime_s(&ptm, &rawtime);
	return ptm;
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
	uint32_t channel;
	uint32_t rssi;
	uint32_t sequence_ctrl;
	uint32_t timestamp;
	uint32_t ssid_length;
	char mac_addr[MAC_LENGTH];
	char ssid[MAX_SSID_LENGTH + 1];
	char hash[MD5_HASH_LENGTH + 1];

	uint32_t *buf_int = (uint32_t *)buf;
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

	for (uint32_t i = 0; i < MAC_LENGTH; i++) {
		mac_addr[i] = *buf_c;
		buf_c++;
	}

	for (uint32_t i = 0; i < MD5_HASH_LENGTH; i++) {
		hash[i] = *buf_c;
		buf_c++;
	}
	hash[MD5_HASH_LENGTH] = '\0';

	for (uint32_t i = 0; i < ssid_length; i++) {
		ssid[i] = *buf_c;
		buf_c++;
	}
	ssid[ssid_length] = '\0';

	/*printf("CH=%d, RSSI=%02d, MAC=%02x:%02x:%02x:%02x:%02x:%02x, SSID=%s, SEQ=%d, TIMESTAMP: %d, HASH=%s\n",
		channel,
		rssi,
		(uint8_t)mac_addr[0], (uint8_t)mac_addr[1], (uint8_t)mac_addr[2],
		(uint8_t)mac_addr[3], (uint8_t)mac_addr[4], (uint8_t)mac_addr[5],
		ssid,
		sequence_ctrl,
		timestamp,
		hash
	);*/

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