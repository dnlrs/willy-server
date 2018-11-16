
#ifndef UNICODE
#define UNICODE 1
#endif


#include "stdafx.h"
#include <iostream>
#include <algorithm> /* std::foreach */
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstdlib> /* needed for _wtoi */
#include <stdexcept>
#include <memory>
#include <winsqlite/winsqlite3.h>
#include "Net_exceptions.h"
#include "packet.h"
#include "Receiver.h"
#include "Conf.h"
#include "Conf_exceptions.h"
#include "Dealer.h"
#include "Database.h"

// Link with all the need libs (this works only for the msvc compiler)
#pragma comment(lib, "Ws2_32.lib")   //winSock library
#pragma comment(lib, "iphlpapi.lib") //windows IP Helper API
#pragma comment(lib, "sqlite3.lib")  //sqlite3 library


HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
std::string conf_folder("config/");
using namespace std;


void read_board_data(Receiver& receiver, mutex& printMtx)
{
	PACKET_T pack;
	while (1)
	{
		try
		{
			pack = receiver();
			struct tm ptm = epochTotm(pack.timestamp);
			//debugging print
			printMtx.lock();
			cout << "--[RECEIVED]-- " << "THREAD ID: " << this_thread::get_id() << endl <<
				"FROM BOARD: " << mactos(receiver.m_mac()) << ", HASH: " << pack.hash << endl <<
				"SSID: " << pack.ssid << ", RSSI: " << pack.rssi << ", MAC: " << mactos(pack.mac_addr) << 
				" CHANNEL: " << to_string(pack.channel) << ", SEQUENCE: " << pack.sequence_ctrl << endl <<
				"TIMESTAMP: -unix: " << pack.timestamp << "\t -readable: " << timetos(ptm) << endl <<
				"--------------------------------------------------------------------------------"<< endl;
			printMtx.unlock();

			/* ==========================
					DB INSERTION
			   ========================== */

		}
		catch (Sock_exception& sock_e) //disconnected receiver(INVALID SOCKET)
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << sock_e.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);

			/* Check if the dealer have fixed the socket */
			if (receiver.has_valid_socket())
				continue;
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << sock_e.what() << " : " << "Impossible to contact board #" << mactos(receiver.m_mac()) << endl;
			SetConsoleTextAttribute(hConsole, 15);
			exit(-1);

		}
		catch (Recv_exception& recv_e) //connection closed by the peer
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << recv_e.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);
			return;
		}
	}
}


							/* ========================================================
													MAIN
							   ======================================================== */

int main()
{
	WSADATA wsaData;
	int i;
	
	vector<Receiver> receivers;
	// Set console output color to default white
	SetConsoleTextAttribute(hConsole, 15);
	// Initialize Winsock.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
	}

	/* ======================================================
			FILE.CONF READING AND BOARDS INITIALIZATION
	   ====================================================== */

	Conf conf(conf_folder+"file.conf"); // TODO: Unhandled exception!!
	cout << "--[BOARDS' LIST]--" << endl;
	for (i = 0; i < conf.m_recvn(); i++)
	{
		try
		{
			conf(); /* reads the next receiver info */
			receivers.push_back(Receiver(conf.m_mac(), conf.m_x(), conf.m_y()));
		}
		catch (Conf_exception& conf_e)
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << conf_e.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);
			exit(-1);
		}

	}

   //Dealer initialization
	Dealer dealer(receivers);


	/* =============================================
			SETUP CONNECTIONS TO ALL THE BOARDS
	   ============================================= */

	try 
	{
		dealer.setup_listeningS();
		dealer.connect_to_all(); /* open first connections */
	}
	catch (Sock_exception& sock_e)
	{
		SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
		cout << sock_e.what() << endl;
		SetConsoleTextAttribute(hConsole, 15);
		WSACleanup();
		exit(-1);
	}
	

	/* =============================================
				RECEIVING DATA THREADS
	   ============================================= */
	vector<thread> thrds;
	for (int i = 0; i < conf.m_recvn(); i ++)
		thrds.push_back(thread(read_board_data, ref(receivers[i]), ref(dealer.getprintMtx())));

	/* =============================================
				LISTENING MAIN THREAD
	============================================= */
	thread listening_thread(&Dealer::accept_incoming_req, &dealer);



	for (int i = 0; i < conf.m_recvn(); i++)
		thrds[i].join();
	thrds.clear();

	
	WSACleanup();
	return 0;
}

