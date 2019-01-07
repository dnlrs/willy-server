
#ifndef UNICODE
#define UNICODE 1
#endif


#include "stdafx.h"
#include <algorithm> /* std::foreach */
#include <stdexcept>
#include "Conf.h"
#include "Dealer.h"
#include "Database.h"
#include "db_exception.h"

// Link with all the need libs (this works only for the msvc compiler)
#pragma comment(lib, "Ws2_32.lib")   //winSock library
#pragma comment(lib, "iphlpapi.lib") //windows IP Helper API
#pragma comment(lib, "sqlite3.lib")  //sqlite3 library


/*
 *	If something bad happens a fatal error notification could be sent both by the receiving threads and by the listening thread.
 *	In this case all the server's threads will terminate and the server will shutted down
 */

#define MAX_RECONNECTION_ATTEMPTS 5 // max number of consecutive reconnection (3-way-handshake without effective data sending)
std::string conf_folder("config/");
std::string end_mess("\n\t*...<server closed>...*\t\n");
using namespace std;


void read_board_data(Receiver& receiver, mutex& printMtx, Dealer& dealer, Database& db)
{
	printMtx.lock();
	cout << "-- [RECEIVING THREAD] -- THREAD ID :" << this_thread::get_id() << ": receiving thread is active for board <"<< mactos(receiver.m_mac()) << ">" << endl;
	printMtx.unlock();

	PACKET_T pack;
	uint32_t attempts = 0;
	while (1)
	{
		try
		{
			pack = receiver();
			struct tm ptm = epochTotm(pack.timestamp);
			//debugging print
			printMtx.lock();
			cout << "-- [RECEIVED] -- " 
                 << "THREAD ID: "    << this_thread::get_id() << endl 
                 << "\tFROM BOARD: " << mactos(receiver.m_mac()) 
                 << ", HASH: " << pack.hash << endl 
                 << "\tSSID: " << pack.ssid 
                 << ", RSSI: " << pack.rssi 
                 << ", MAC: "  << mactos(pack.mac_addr) 
                 << "CHANNEL: " << to_string(pack.channel) 
                 << ", SEQUENCE: " << pack.sequence_ctrl << endl 
                 << "\tTIMESTAMP: -unix: " << pack.timestamp 
                 << "\t -readable: " << timetos(ptm) << endl 
                 << "--------------------------------------------------------------------------------"<< endl;
			printMtx.unlock();

            db.add_packet(pack, receiver.m_mac().compacted_mac);
			
		}
		catch (Sock_exception& sock_e) //disconnected receiver(INVALID SOCKET)
		{
			lock_guard<mutex> lg(printMtx);
			cerr << sock_e.what() << endl;
			if (dealer.in_err())
			{
				cerr << sock_e.what() << endl;
				cout << "Fatal error catched, Thread #" << this_thread::get_id() 
					<< " serving board <" << mactos(receiver.m_mac()) << "> exiting" << endl;
				return;
			}

			/* Check if the dealer have fixed the socket */
			if (receiver.has_valid_socket())
			{
				attempts = 0; //reset the attempts for the current reconnection
				continue;
			}
				
			else
				attempts++;
			if (attempts >= MAX_RECONNECTION_ATTEMPTS)
			{
				dealer.notify_fatal_err();
				cerr << sock_e.what() << " : " << "Impossible to contact board <" << mactos(receiver.m_mac()) << ">. Fatal error notification has been sent" << endl;
				return;
			}
		}
		catch (Recv_exception& recv_e) //connection closed by the peer
		{
			dealer.notify_fatal_err(); //inform the other threads to exit
			lock_guard<mutex> lg(printMtx);
			cerr << recv_e.what() << endl;
			return;
		}
		catch (db_exception& db_e) //connection closed by the peer
		{
			dealer.notify_fatal_err(); //inform the other threads to exit
			lock_guard<mutex> lg(printMtx);
			cerr << db_e.what() << endl;
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
	// Initialize Winsock.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
	}

	/* ======================================================
			FILE.CONF READING AND BOARDS INITIALIZATION
	   ====================================================== */
	Conf conf;
	try
	{
		conf.load_conf(conf_folder + "file.conf");
	}
	catch (Conf_exception& conf_e)
	{
		cerr << conf_e.what() << endl << end_mess << endl;
		system("pause");
		exit(-1);
	}
	cout << "-- [BOARDS' LIST] --" << endl;
	for (i = 0; i < conf.m_recvn(); i++)
	{
		try
		{
			conf(); /* reads the next receiver info */
			receivers.push_back(Receiver(conf.m_mac(), conf.m_x(), conf.m_y()));
		}
		catch (Conf_exception& conf_e)
		{
			cerr << conf_e.what() << endl << end_mess << endl;
			system("pause");
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
		cerr << sock_e.what() << endl << end_mess << endl;
		WSACleanup();
		system("pause");
		exit(-1);
	}
	
    /* =============================================
                DB INITIALIZATION
    ============================================= */
    Database db("database.db", conf.m_recvn());
    db.init();


	/* =============================================
				RECEIVING DATA THREADS
	   ============================================= */
	vector<thread> thrds;
	for (int i = 0; i < conf.m_recvn(); i ++)
		thrds.push_back(thread(read_board_data, ref(receivers[i]), ref(dealer.getprintMtx()), ref(dealer), ref(db)));

	/* =============================================
				REQUESTS LISTENING THREAD
	============================================= */
	thread listening_thread(&Dealer::accept_incoming_req, &dealer);


	for (int i = 0; i < conf.m_recvn(); i++)
		thrds[i].join();
	thrds.clear();

	dealer.close_listening(); //close the listening socket to unlock listening thread from the accept()
	listening_thread.join();
	
	cout << "... SERVER SHUTTED DOWN ..." << endl << end_mess << endl;
	WSACleanup();
	return 0;
}

