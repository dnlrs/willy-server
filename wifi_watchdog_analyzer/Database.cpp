#include "Database.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;



Database::Database(string s, int n)
{
	sqlite3 *db;
	const char * nomeDB = s.c_str();
	if (sqlite3_open( nomeDB, &db) != SQLITE_OK)
	{
		cerr << sqlite3_errmsg(db);
	}
	else
	{
		cout << "Connessione stabilita";
	}
	data = db;
	dispositivi = n;
	
}

Database::Database()
{
}

Database::~Database()
{
}

void Database::InitTabelle() //una tantum, inizializzazione tabelle
{
	sqlite3_stmt * stmt;
	string sql1 = "CREATE TABLE PACKET(HASH TEXT NOT NULL, SSID TEXT, RSSI INTEGER, MAC TEXT, CHANNEL INTEGER, TIMESTAMP INTEGER NOT NULL, SEQUENCE_NUMBER INTEGER, MACDEVICE TEXT NOT NULL, PRIMARY KEY(HASH,MACDEVICE));";
	
	sqlite3_prepare(data, sql1.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "tabella creata\n";
	}
	sqlite3_finalize(stmt);

	string sql2 = "CREATE TABLE DEVICE (MAC TEXT ,TIMESTAMP INTEGER , X REAL, Y REAL, PRIMARY KEY(MAC, TIMESTAMP));";
	sqlite3_prepare(data, sql2.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "tabella creata\n";
	}
	sqlite3_finalize(stmt);

}


void Database::Insert_packet(string s /*packet_t pack*/) //inserimento pacchetti
{
	sqlite3_stmt *stmt;

	/*inserire il parsing che si trova nel main*/
	
	/*cambiare il +s con il nome della stringa parsata*/
	string sql = "INSERT INTO PACKET VALUES(" + s + ")";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "ok\n";
	}
	sqlite3_finalize(stmt);

	string sql2 = "SELECT COUNT(*) FROM PACKET WHERE MAC = '-80,2c:ae:2b:d3:ba:a9' AND TIMESTAMP > 1527269530 AND TIMESTAMP < 1527269532 GROUP BY HASH HAVING COUNT(DISTINCT MACDEVICE) = " +to_string(dispositivi) + ";" ;
//	string sql2 = "SELECT COUNT(*) FROM PACKET WHERE MAC = "+ pack.mac+ " AND TIMESTAMP > "+to_string(pack.timestamp -2)+" AND TIMESTAMP < "+to_string(pack.timestamp +2) GROUP BY HASH, MACDEVICE HAVING COUNT(DISTINCT MACDEVICE) = " +to_string(dispositivi) + ";" ;
/*scommentare il secondo, commentare il primo*/
	sqlite3_prepare(data, sql2.c_str(), -1, &stmt, NULL);
	int a = sqlite3_step(stmt);
	if (a == SQLITE_ROW) {
		if (sqlite3_column_int(stmt, 0) != dispositivi) {
			cout << "Pacchetto non ricevuto da tutti";
		return;
		}
		else {
			cout << "Pacchetto ricevuto da tutti";
			CompletaPacchetto("-80,2c:ae:2b:d3:ba:a9", 1527269531);
			/*CompletaPacchetto("pack.mac, pack.timestamp");*/
			/*commenta primo, scommenta secondo*/
		}
	}
}

void Database::Delete(string s) //pulizia db precedente a un certo timestamp
{
	sqlite3_stmt *stmt;
	string sql = "DELETE FROM PACKET WHERE TIMESTAMP < " + s + ";" ;
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "ok\n";
	}
	sqlite3_finalize(stmt);
}

void Database::Refresh() //azzeramento tabella dei pacchetti
{
	sqlite3_stmt *stmt;
	string sql = "DELETE FROM PACKET;";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "ok\n";
	}
	sqlite3_finalize(stmt);
}

void Database::ChiudiConn()
{
	sqlite3_close(data);
}

vector<Timestamps> Database::Statistiche_lungoPeriodo(string limit, string init, string fine, int &flag)
{
	vector<Timestamps> vett;
	int i = 0, count;
	sqlite3_stmt *stmt;
	int a = -10;
	string sql = "SELECT MAC, COUNT(*) FROM DEVICIE WHERE TIMESTAMP >= " + init + " AND TIMESTAMP <= " + fine + " GROUP BY MAC ORDER BY COUNT(*) DESC LIMIT " + limit + ";";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);

	while (a != SQLITE_DONE ) {
		
		Timestamps tmp;
		char * str = (char*)sqlite3_column_text(stmt, 0);
		char * str2 = (char *)sqlite3_column_text(stmt, 1);
		tmp.mac = str;
		count = atoi(str2);
		tmp.num = count;
		tmp.tempi.resize(count);
		vett.push_back(tmp);
		a = sqlite3_step(stmt);
		i++;

		
	}
	sqlite3_finalize(stmt);
	flag = i;
	for (int j = 0; j < i; j++) {
		vett[j]=RiempiTimestamp(vett[j],init,fine);
	}

	
	return vett;
	
}

vector<Movimenti> Database::Statistiche_Movimento(string init, string fine)
{
	sqlite3_stmt *stmt;
	int a = -10, i = 0, count;
	vector<Movimenti> vett;
	string sql = "SELECT MAC, COUNT(*) FROM DEVICIE WHERE TIMESTAMP >= " + init + " AND TIMESTAMP <= " + fine + " GROUP BY MAC;";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);
	while (a != SQLITE_DONE) {
		Movimenti tmp;
		char * str = (char*)sqlite3_column_text(stmt, 0);
		char * str2 = (char*)sqlite3_column_text(stmt, 1);
		tmp.mac = str;
		count = atoi(str2);
	    tmp.occur = count;
		tmp.posX.resize(count);
		tmp.posY.resize(count);
		i++;
		vett.push_back(tmp);
		a = sqlite3_step(stmt);
		
	}
	for (int j = 0; j < i; j++) {
		vett[j] = RiempiMovimenti(vett[j], init, fine);
		}
	return vett;
}



void Database::Insert_device(string d)
{
	sqlite3_stmt *stmt;
	string sql = "INSERT INTO DEVICE VALUES(" + d + ");";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_DONE) {
		cout << "ok\n";
	}
	sqlite3_finalize(stmt);
}

list<string> Database::Device_connessi(string time)
{
	list<string> connessi;
	sqlite3_stmt *stmt;
	int a = -10;
	string sql = "SELECT DISTINCT MAC FROM DEVICE WHERE TIMESTAMP >= " + time + ";";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);
	while (a != SQLITE_DONE) {
		char * str = (char*)sqlite3_column_text(stmt, 0);
		connessi.push_back(str);
		a = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	return connessi;
}

vector<Pacchetto_c> Database::getPacchi()
{
	vector<Pacchetto_c> tmp;
	tmp = pacchetti;
	svuotaVector();
	return tmp;
}

Timestamps Database::RiempiTimestamp(Timestamps t, string init, string fine)
{
	
	sqlite3_stmt *stmt;
	int a = -10, i = 0;
	int tmp = 0;
	string sql = "SELECT TIMESTAMP FROM DEVICIE WHERE TIMESTAMP >= " + init + " AND TIMESTAMP <= " + fine + " AND MAC = '" + t.mac + "';";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);
	while (a != SQLITE_DONE) {
		char * str = (char*)sqlite3_column_text(stmt, 0);
		tmp = atoi(str);
		t.tempi[i] = tmp;
		a = sqlite3_step(stmt);
		i++;
	}
	sqlite3_finalize(stmt);
	return t;
}

Movimenti Database::RiempiMovimenti(Movimenti m, string init, string fine)
{
	sqlite3_stmt *stmt;
	int a = -10, i = 0;
	float tmp1, tmp2;
	string sql = "SELECT X,Y FROM DEVICIE WHERE TIMESTAMP >= " + init + " AND TIMESTAMP <= " + fine + " AND MAC = '" + m.mac + "';";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);
	while (a != SQLITE_DONE) {
		char * str = (char*)sqlite3_column_text(stmt, 0);
		char * str2 = (char*)sqlite3_column_text(stmt, 1);
		tmp1 = std::stof(str);
		tmp2 = std::stof(str2);
//		tmp1 = atof(str);
//		tmp2 = atof(str2);
		m.posX[i] = tmp1;
		m.posY[i] = tmp2;
		a = sqlite3_step(stmt);
		i++;
	}
	sqlite3_finalize(stmt);
	return m;
	
}

void Database::CompletaPacchetto(string mac, int timestamp)
{
	sqlite3_stmt *stmt;
	float avgTimestamp = 0, temp;
	Pacchetto_c pack;
	pack.mac = mac;
	int dist;
	int a = -10;
	string t1 = to_string(timestamp - 2);
	string t2 = to_string(timestamp + 2);
	string sql = "SELECT MACDEVICE, TIMESTAMP, RSSI FROM PACKET WHERE MAC = '" +mac+ "' AND TIMESTAMP > "+t1+ " AND TIMESTAMP < "+t2+ ";";
	sqlite3_prepare(data, sql.c_str(), -1, &stmt, NULL);
	a = sqlite3_step(stmt);
	while (a != SQLITE_DONE) {
		char * str = (char*)sqlite3_column_text(stmt, 0);
		string macDev = str;
		char * str2 = (char*)sqlite3_column_text(stmt, 1);
		temp = stof(str2);
		char * str3 = (char*)sqlite3_column_text(stmt, 2);
		dist = atoi(str3);
		pack.macRssi.push_back(std::make_pair(macDev, dist));
		avgTimestamp += temp;
		a = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	avgTimestamp = avgTimestamp / dispositivi;
	pack.time = avgTimestamp;
	pacchetti.push_back(pack);
}

void Database::svuotaVector()
{
	pacchetti.clear();
}




