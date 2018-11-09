
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
#include <string>
#include <memory>
#include <winsqlite/winsqlite3.h>
#include "Net_exceptions.h"
#include "packet.h"
#include "Receiver.h"
#include "Conf.h"
#include "Conf_exceptions.h"
#include "Dealer.h"
#include "Database.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")


HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

using namespace std;


void read_board_data(Receiver& receiver/* Dealer& dealer*/)
{
	PACKET_T pack;
	string dbvalues;
	while (1)
	{
		try
		{
			pack = receiver();
			dbvalues.clear();
			//set the correct timestamp
			/*pack.timestamp = dealer.synchToUnix(pack.off_timestamp);*/
			//for printing only
			/*SYSTEMTIME st;
			UnixTimeToSystemTime(pack.timestamp, &st);*/
			//debugging print
			cout << "THREAD ID: " << this_thread::get_id() << ", BOARD: " << mactos(receiver.m_mac()) << "|| HASH: " << pack.hash
				<< ", SSID: " << pack.ssid << ", RSSI: " << pack.rssi << ", MAC: " << mactos(pack.mac_addr) << ", CHANNEL: "
				<< to_string(pack.channel) << ", TIMESTAMP: " /*<< timetos(st)*/ << ", SEQUENCE: " << pack.sequence_ctrl << ", OFFSET: " /*<< pack.off_timestamp*/ << endl;

			/* ==========================
					DB INSERTION
			   ========================== */

		}
		catch (Sock_exception& sock_e) //disconnected receiver(INVALID SOCKET). . . if code == 10038 ==> problem with my socket
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << sock_e.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);

			/* retry the connection. Serious error in case of exception during a new connecting attempt: program should be closed */
			try {
				// Request ownership of the critical section.
				//EnterCriticalSection(&CriticalSection); //blocking if the criticalSection is occupied

				/*dealer.reconnecting(receiver);*/

				// Release ownership of the critical section.
				//LeaveCriticalSection(&CriticalSection);
			}
			catch (Sock_exception& sock_e) //recreate the listening socket or close the program?
			{
				//LeaveCriticalSection(&CriticalSection);
				SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
				cout << sock_e.what() << " : " << "Impossible to contact board #" << mactos(receiver.m_mac()) << endl;
				SetConsoleTextAttribute(hConsole, 15);
				exit(-1);  // decrease # of boards and check if the remaining are at least 2. Otherwhise close the program
			}

			/* Put this packet in the list of missing packet and request them again after connection is finished */

		}
		catch (Recv_exception& recv_e)
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << recv_e.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);
			exit(-1);
			/* Put this packet in the list of missing packet and request them again after connection is finished */
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

	Conf conf("file.conf");
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
				DETACH RECEIVING DATA THREADS
	   ============================================= */

	for (Receiver r : receivers)
		thread(read_board_data, ref(r)).detach();
	
	
	//iterate each N seconds on -> data_process();

	


	// Release resources used by the critical section object.
	//DeleteCriticalSection(&CriticalSection);
	WSACleanup();
	return 0;
}

