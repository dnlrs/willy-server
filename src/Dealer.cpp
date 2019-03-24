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
	cout << "Waiting for clients to connect..." << endl;

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

	struct sockaddr_in accAddr;
	socklen_t addrlen;
	list<uint64_t> guest_list;
	for (Receiver r : this->recvs)
		guest_list.push_back(r.m_mac().compacted_mac);
	size_t missingboards = this->recvs.size();
	size_t checkedboard = 0;
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
		if (this->recvs[iResult].has_valid_socket())
			closesocket(this->recvs[iResult].m_sock()); //close socket to avoid multiple accept to leave multiple opened socket
		this->recvs[iResult].set_sock(acceptSocket);

		uint32_t ack = 1;
		size_t bytes = send(recvs[iResult].m_sock(), (char *)&ack, sizeof(uint32_t), 0);
		if (bytes < 0)
			exit(-1);
		cout << "Receiver <" << mactos(recvs[iResult].m_mac()) << "> started." << endl;

		
		//search for the connected board, if this one were already accepted do not count it as a new checkedboard
		auto it = std::find(guest_list.begin(), guest_list.end(), this->recvs[iResult].m_mac().compacted_mac);
		if(it == guest_list.end())
			cout << "++ Receiver <" << mactos(this->recvs[iResult].m_mac()) << "> reconnected. ++" << endl;
		else
		{
			guest_list.erase(it); //remove the new connected board from the guest_list
			cout << "Receiver <" << mactos(this->recvs[iResult].m_mac()) << "> connected." << endl;
			checkedboard++;
		}
	}
	
	//send start to all the anchors
	/*
	uint32_t ack = 1;
	for (Receiver recv : recvs)
	{
		size_t bytes = send(recv.m_sock(), (char *)&ack, sizeof(uint32_t), 0);
		if (bytes < 0)
			exit(-1);
		cout << "Receiver <" << mactos(recv.m_mac()) << "> started." << endl;
	}
	*/
}

void Dealer::accept_incoming_req()
{
	int iResult;
	stringstream fail;

	this->printMtx.lock();
	cout << "-- [LISTENING THREAD] -- THREAD ID :" << this_thread::get_id() << ": listening thread is active" << endl;
	this->printMtx.unlock();

	// Create a SOCKET for accepting incoming requests.
	SOCKET acceptSocket = INVALID_SOCKET;

	//----------------------
	// ARP chache table 	
	unsigned long status;
	PMIB_IPNET_TABLE2 pipTable = NULL;
	status = GetIpNetTable2(AF_INET, &pipTable);
	if (status != NO_ERROR) {
		lock_guard<mutex> lg(this->printMtx);
		printf("GetIpNetTable for IPv4 table returned error: %ld\n", status);
		exit(-1); // radical error
	}

	//----------------------
	// Accept the connection.
	struct sockaddr_in accAddr;
	socklen_t addrlen;

	while (1)
	{
		addrlen = sizeof(struct sockaddr_in);
		//clear the structure
		memset(&accAddr, 0, addrlen);
		acceptSocket = accept(this->listenSocket, (SOCKADDR *)&accAddr, &addrlen);
		if (acceptSocket == INVALID_SOCKET) 
		{
			this->printMtx.lock();
			cout << "-- [ERROR] -- THREAD ID " << this_thread::get_id() << " Accept failed - Listening thread is closing ...";
			this->printMtx.unlock();
			if (!this->in_err())
				this->notify_fatal_err();
			return;
		}

		/* check if this ip corresponds to a valid board and insert the AcceptSocket into the correspondant Receiver object */
		iResult = check_if_valid_board(accAddr.sin_addr.S_un.S_addr, pipTable, this->recvs, this->recvs.size());
		if (iResult == -1) //not valid board
		{
			closesocket(acceptSocket);
			continue;
		}
		//sets the new socket
		SOCKET old_sock = this->recvs[iResult].m_sock();
		this->recvs[iResult].set_sock(acceptSocket);
		//closes socket to unlock the thread blocked on the recv()
		closesocket(old_sock);
		
		uint32_t ack = 1;
		size_t bytes = send(acceptSocket, (char *)&ack, sizeof(uint32_t), 0);
		if (bytes <= 0)
		{
			this->printMtx.lock();
			cout << "-- [ERROR] -- THREAD ID " << this_thread::get_id() << " Ack sending failed - Listening thread is closing ...";
			this->printMtx.unlock();
			if (!this->in_err())
				this->notify_fatal_err();
			return;
		}
		this->printMtx.lock();
		cout << "-- [RECONNECTION] -- THREAD ID " << this_thread::get_id() << endl
			<< "\t\tReceiver <" << mactos(this->recvs[iResult].m_mac()) << "> reconnected and started." << endl;
		this->printMtx.unlock();
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
				return (int)rentry;
		}
	}
	return -1;
}

void Dealer::notify_fatal_err()
{
	lock_guard<mutex> lg(this->fatalErrMtx);
	if (this->fatal_error)
		return;
	this->fatal_error = true;
	//close all receivers socket to unlock them from the recv(). The closesocket() is blocked until all the incoming data has been received
	for (Receiver r : this->recvs)
		closesocket(r.m_sock());
}

boolean Dealer::in_err()
{
	lock_guard<mutex> lg(this->fatalErrMtx);
	return this->fatal_error;
}

std::map<uint64_t, Point2d> Dealer::get_anchor_positions()
{
    std::map<uint64_t, Point2d> rval;

    for (Receiver r : recvs)
        rval[r.m_mac().compacted_mac] = r.m_loc();
    
    return rval;
}