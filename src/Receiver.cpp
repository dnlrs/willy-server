
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
	uint32_t pack_size;
	SSIZE_T bytes;
	string fail = string("recv() failed");

	bytes = recv(this->sock, (char *)&pack_size, sizeof(uint32_t), 0);
	if(bytes == 0) //connection closed by peer
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (bytes < 0) //invalid socket or connection closed
		throw Sock_exception(fail);
	
	char* recv_buff = nullptr;
	recv_buff = (char *)malloc(pack_size * sizeof(char));
	if (recv_buff == nullptr)
	{
		cout << "memory error" << endl;
		exit(-1);
	}

	cout << "PACK SIZE: " << pack_size << endl;
	bytes = recv(this->sock, recv_buff, pack_size, 0);
	if (bytes == 0) //connection closed by peer
		throw Recv_exception(fail.append(": connection closed by peer"));
	else if (bytes < 0) //invalid socket or connection closed
		throw Sock_exception(fail);

	PACKET_T pack = deserialize(recv_buff);
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
