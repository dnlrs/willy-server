
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
	//int tot_pack = receiver.total_packet();
	//int currpack = 0;
	string dbvalues;
	while (1)
	{
		try
		{
			pack = receiver();
			//currpack++;
			dbvalues.clear();
			//set the correct timestamp
			/*pack.timestamp = dealer.synchToUnix(pack.off_timestamp);*/
			//for printing only
			/*SYSTEMTIME st;
			UnixTimeToSystemTime(pack.timestamp, &st);*/
			//debugging print
			cout << "THREAD ID: " << this_thread::get_id() << ", BOARD: " << mactos(receiver.m_mac()) << "|| HASH: " << pack.hash
				<< ", SSID: " << pack.ssid << ", RSSI: " << pack.rssi << ", MAC: " << mactos(pack.mac_addr) << ", CHANNEL: "
				<< to_string(pack.channel) << ", TIMESTAMP: " /*<< timetos(st)*/ << ", SEQUENCE: " << pack.seq_number << ", OFFSET: " /*<< pack.off_timestamp*/ << endl;
			/*if(pack.ssid.empty())
				dbvalues = "'" + pack.hash + "', '" + "NULL" + "', '" + to_string(pack.rssi) + "', '" + mactos(pack.mac_addr) + "', '" + to_string(pack.channel) + "', '" + to_string(pack.timestamp) + "', '" + to_string(pack.seq_number) + "', '" + mactos(receiver.m_mac()) + "'";
			else
				dbvalues = "'" + pack.hash + "', '" + pack.ssid + "', '" + to_string(pack.rssi) + "', '" + mactos(pack.mac_addr) + "', '" + to_string(pack.channel) + "', '" + to_string(pack.timestamp) + "', '" + to_string(pack.seq_number) + "', '" + mactos(receiver.m_mac()) + "'";
			*/
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



void manage_conn(vector<Receiver>& receivers)
{ 
	/* =================================
			OPEN LISTENING SOCKET
	   ================================= */

	Dealer dealer(receivers);
	try
	{
		dealer.setup_listening();
	}
	catch (Sock_exception& sock_e)
	{
		SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
		cout << sock_e.what() << endl;
		SetConsoleTextAttribute(hConsole, 15);
		WSACleanup();
		exit(-1);
	}

	/* =================================
			OPEN FIRST CONNECTION
	   ================================= */

	int retry = 2;
	/* check for all receiver if they are present */
	while (retry > 0)
	{
		try
		{
			dealer.setup_all(); /* open the first connection with all the missing boards */
			break;
		}
		catch (Sock_exception& s_exc)
		{
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
			cout << s_exc.what() << endl;
			SetConsoleTextAttribute(hConsole, 15);
			retry--;
			if (!retry)
			{
				WSACleanup();
				throw;
			}

		}
	}
}


void manage_reception(vector<Receiver>& receivers)
{
	vector<thread> thr;
	while (1)
	{
		/* ====================================
				BOARDS DATA READING
		   ==================================== */


		// receives data from the receivers (one thread per receiver). The first time the container will be void (no synch needed before)
		//read_board_data(receivers[0]); //debug
		for(Receiver r : receivers)
			thr.push_back(thread(read_board_data, ref(r)));
		for (unsigned int i = 0; i < thr.size(); i ++)
			thr[i].join();
		thr.clear();


		/* ====================================
					SYNCHRONIZATION
		   ==================================== */

		//send the ack to the boards and then saves the local time
		/*dealer.synch();
		time_t deb_time = dealer.synchToUnix(0); //here it returns simply the actual time
		SYSTEMTIME st;
		UnixTimeToSystemTime(deb_time, &st);
		cout << "--------------------------------------- LOCAL TIME: " << timetos(st) << "--------------------------------------- " << endl;
		*/
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

   
	manage_conn(receivers); /* open first connection */
	thread data_receiver(manage_reception, ref(receivers));
	//data_process();

	


	// Release resources used by the critical section object.
	//DeleteCriticalSection(&CriticalSection);
	WSACleanup();
	return 0;
}

