
#include "stdafx.h"
#include "Receiver.h"


Receiver::~Receiver()
{
	/* destructor cannot throw exceptions */
	closesocket(this->sock);
}


/* Define a data id to make possible a resend in case of error (??)*/
PACKET_T Receiver::operator() ()
{
	string hash_buff(HASH_LENGHT, 0),
					  ssid_buff(SSID_LENGHT, 0);
	int rssi;
	UINT32 seq_number;
	UINT8 channel,
			ssid_len;
	mac_t macaddr;
	UINT16 off_time;
	SSIZE_T byte;

	string fail = string("recv() failed");
	
	// channel
	byte = recv(this->sock, (char*)&channel, sizeof(UINT8), 0);
	if (byte == 0) //connection closed by peer
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0) //invalid socket or connection closed
		throw Sock_exception(fail);

	// hash
	byte = recv(this->sock, &hash_buff[0], HASH_LENGHT, 0);
	if (byte == 0)
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0)
		throw Sock_exception(fail);

	// mac
	for (int i = 0; i < MAC_BYTES; i++)
	{
		byte = recv(this->sock, (char*)&(macaddr.raw_mac[i]), sizeof(unsigned char), 0);
		if (byte == 0)
			throw Recv_exception(fail.append(": connection closed by peer"));
		else if (byte < 0)
			throw Sock_exception(fail);
	}	

	// rssi
	byte = recv(this->sock, (char*)&rssi, sizeof(int), 0);
	if (byte == 0)
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0)
		throw Sock_exception(fail);

	// sequence number
	byte = recv(this->sock, (char*)&seq_number, sizeof(UINT32), 0);
	if (byte == 0)
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0)
		throw Sock_exception(fail);

	//ssid len
	byte = recv(this->sock, (char*)&ssid_len, sizeof(UINT8), 0);
	if (byte == 0)
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0)
		throw Sock_exception(fail);
	
	// ssid
	if (ssid_len > 0)
	{
		byte = recv(this->sock, &ssid_buff[0], ssid_len, 0);
		if (byte == 0)
			throw Recv_exception(fail.append(": connection closed by peer"));
		else if (byte < 0)
			throw Sock_exception(fail);
	}
	ssid_buff.resize(ssid_len);

	// raw timestamp
	byte = recv(this->sock, (char*)&off_time, sizeof(UINT16), 0);
	if (byte == 0)
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (byte < 0)
		throw Sock_exception(fail);

	PACKET_T pack;
	pack.channel = channel;
	pack.hash = string(hash_buff);
	pack.ssid = string(ssid_buff);
	pack.rssi = ntohl(rssi);
	pack.mac_addr = macaddr;
	pack.off_timestamp = ntohs(off_time); //receiver can sets the offset only
	pack.seq_number = ntohs(seq_number);

	return pack;
}

int Receiver::total_packet()
{
	int numpack;
	SSIZE_T byte;
	byte = recv(this->sock, (char*)&numpack, sizeof(int), 0);
	if (byte == -1)
		wprintf(L"recv() failed with error: %ld\n", WSAGetLastError());
	numpack = ntohs(numpack);
	return numpack;
}
