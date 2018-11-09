#include "stdafx.h"
#include "Dealer.h"



void Dealer::setup_listeningS()
{
	stringstream fail;
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.

	this->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		fail << "socket failed with error: " << WSAGetLastError() << " -- [Listening Socket]";
		throw Sock_exception(fail.str());
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(SERVICE_PORT);

	if (::bind(this->listenSocket,
		(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
		this->listenSocket = INVALID_SOCKET; //reset the socket
		fail << "bind failed with error: " << WSAGetLastError() << " -- [Listening Socket]";
		closesocket(listenSocket);
		throw Sock_exception(fail.str());
	}
	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(this->listenSocket, 1) == SOCKET_ERROR) {
		this->listenSocket = INVALID_SOCKET; //reset the socket
		fail << "listen failed with error: " << WSAGetLastError() << " -- [Listening Socket]";
		closesocket(listenSocket);
		throw Sock_exception(fail.str());
	}
}

void Dealer::connect_to_all()
{
	int iResult;
	stringstream fail;

	// Create a SOCKET for accepting incoming requests.
	SOCKET acceptSocket = INVALID_SOCKET;
	wprintf(L"Waiting for client to connect...\n");

	//----------------------
	// ARP chache table 	
	unsigned long status;
	PMIB_IPNET_TABLE2 pipTable = NULL;
	status = GetIpNetTable2(AF_INET, &pipTable);
	if (status != NO_ERROR) {
		printf("GetIpNetTable for IPv4 table returned error: %ld\n", status);
		exit(-1); // radical error
	}

	//----------------------
	// Accept the connection.
	int missingboards = 0;
	for (Receiver rec : this->recvs)
	{ 
		//if (rec.m_sock() == INVALID_SOCKET)
			missingboards++;
	}
	int checkedboard = 0;
	struct sockaddr_in accAddr;
	socklen_t addrlen;

	// ------------------------------ **************** here send broadcast  ******************

	while (checkedboard < missingboards)
	{
		addrlen = sizeof(struct sockaddr_in);
		//clear the structure
		memset(&accAddr, 0, addrlen);
		acceptSocket = accept(this->listenSocket, (SOCKADDR *)&accAddr, &addrlen);
		if (acceptSocket == INVALID_SOCKET) {
			fail << "accept failed";
			throw Sock_exception(fail.str());
		}

		/* check if this ip corresponds to a valid board and insert the AcceptSocket into the correspondant Receiver object */
		iResult = check_if_valid_board(accAddr.sin_addr.S_un.S_addr, pipTable, this->recvs, this->recvs.size());
		if (iResult == -1) //not valid board
		{
			closesocket(acceptSocket);
			continue;
		}
		this->recvs[iResult].set_sock(acceptSocket);
		wprintf(L"Receiver connected.\n");
		checkedboard++;

	}
}

int Dealer::check_if_valid_board(const u_long& ip, const PMIB_IPNET_TABLE2& arpTable, const vector<Receiver>& receivers, const size_t nrecv)
{
	SIZE_T rentry,
		currbyte;
	for (ULONG currentry = 0; currentry < arpTable->NumEntries; currentry++)
	{
		if (ip != (arpTable->Table[currentry].Address.Ipv4.sin_addr.S_un.S_addr))
			continue;
		for (rentry = 0; rentry < nrecv; rentry++)
		{
			for (currbyte = 0; currbyte < MAC_LENGTH && currbyte < (unsigned char)arpTable->Table[currentry].PhysicalAddressLength; currbyte++)
				if (receivers[rentry].m_mac().raw_mac[currbyte] != (unsigned char)arpTable->Table[currentry].PhysicalAddress[currbyte])
					break;
			if (currbyte == 6)  // correspondant board was found
				return rentry;
		}
	}
	return -1;
}

void Dealer::reconnecting(Receiver& r) //@TODO: adjust
{
	// search at first if a pending requests for that receiver exists
	auto it = this->pendingRequests.find(mactos(r.m_mac()));
	if (it != this->pendingRequests.end()) //request found!
	{
		r.set_sock(this->pendingRequests[mactos(r.m_mac())]);
		this->pendingRequests.erase(mactos(r.m_mac())); //delete the request
	}

	//if no already accepted requests then wait for it
	SOCKET acceptSock = INVALID_SOCKET;
	struct sockaddr_in accAddr;
	socklen_t addrlen;
	stringstream fail;
	int iResult;


	// ARP chache table 	
	unsigned long status;
	PMIB_IPNET_TABLE2 pipTable = NULL;
	status = GetIpNetTable2(AF_INET, &pipTable);
	if (status != NO_ERROR) {
		printf("GetIpNetTable for IPv4 table returned error: %ld\n", status);
		exit(-1); // radical error
	}


	while (1)
	{
		accept(this->listenSocket, (SOCKADDR *)&accAddr, &addrlen);
		if (acceptSock == INVALID_SOCKET) {
			fail << "accept failed";
			throw Sock_exception(fail.str());
		}
		iResult = check_if_valid_board(accAddr.sin_addr.S_un.S_addr, pipTable, this->recvs, this->recvs.size());
		if (iResult == -1) //not valid board
		{
			closesocket(acceptSock);
			continue;
		}
		
		if (this->recvs[iResult] == r)
			r.set_sock(acceptSock);
		else
			this->pendingRequests[mactos(this->recvs[iResult].m_mac())] = acceptSock; //add to pending requests
	}


}

void Dealer::synch()
{
	char a = 'a';
	int byte = -1;
	//uint8_t d = 6; // ACK
	//d = htons(d);
	//for (Receiver r : receivers) // @TODO: threads creation ?
	for(SIZE_T i = 0; i < this->recvs.size(); i ++)
	{
		byte = send(this->recvs[i].m_sock(), &a, sizeof(char), 0); //unlock receivers from the pause
		if (byte == -1)
			cout << "Error: " << WSAGetLastError() << endl;
	}
	GetLocalTime(&(this->lastSynch));
}

time_t Dealer::synchToUnix(UINT16 off)
{
	FILETIME f;
	SystemTimeToFileTime(&(this->lastSynch), &f);
	time_t unix_t = this->FileTime_to_POSIX(f);
	unix_t += off;
	return unix_t;




	/*TIME_T tmp;
	//seconds & min adjusting
	tmp.second = this->lastSynch.wSecond;
	WORD new_min = off / 60;
	tmp.second -= new_min * 60;

	tmp.minute = this->lastSynch.wMinute;
	tmp.minute += new_min;
	//minutes & hours adjusting
	WORD new_hours = tmp.minute / 60;
	tmp.minute -= new_hours * 60;

	tmp.hour = this->lastSynch.wHour;
	tmp.hour += new_hours;
	//hours and day adjusting
	WORD new_days = tmp.hour / 24;
	tmp.hour -= new_days * 24;

	tmp.day = this->lastSynch.wDay;
	tmp.day += new_days;
	//day and month adjusting
	WORD new_months = tmp.day / 30;   // @TODO: 30 or 31 ??? ( month31[i] == 1 if month I has got 31 day) 
	tmp.day -= new_months * 30;

	tmp.month = this->lastSynch.wMonth;
	tmp.month += new_months;
	//month and year adjusting
	WORD new_years = tmp.month / 30;
	tmp.month -= new_years * 30;

	tmp.year = this->lastSynch.wYear;
	tmp.year += new_years;
	return tmp;*/
}

time_t Dealer::FileTime_to_POSIX(FILETIME ft)
{

		// takes the last modified date

		LARGE_INTEGER date, adjust;
		date.HighPart = ft.dwHighDateTime;
		date.LowPart = ft.dwLowDateTime;
		// 100-nanoseconds = milliseconds * 10000

		adjust.QuadPart = 11644473600000 * 10000;
		// removes the diff between 1970 and 1601

		date.QuadPart -= adjust.QuadPart;
		// converts back from 100-nanoseconds to seconds
		return date.QuadPart / 10000000;
}