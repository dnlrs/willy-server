#pragma once
#ifndef DATABASE
#define DATABASE

#include<stdio.h>
#include<stdlib.h>
#include <winsqlite/winsqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>


using namespace std;
typedef struct {
	string mac;
	int num;
	vector<int> tempi;
}Timestamps;

typedef struct {
	string mac;
	int occur;
	vector<float> posX;
	vector<float> posY;
}Movimenti;

typedef struct {
	string mac;
	float time;
	vector<pair<string, int>> macRssi;
}Pacchetto_c;


class Database
{
public:

	Database();
	Database(string,int);
	~Database();
	void InitTabelle();
	void Insert_packet(string v);
	void Delete(string n /*eventuale tempo*/); /*elima tutti i pacchetti antecedenti a un certo periodo */
	void Refresh(); //svuota db;
	void ChiudiConn();
	vector<Timestamps> Statistiche_lungoPeriodo(string,string,string,int&);
	vector<Movimenti> Statistiche_Movimento(string,string);
	void Insert_device(string d);
	list<string> Device_connessi(string);
	vector<Pacchetto_c> getPacchi();

private:
	sqlite3 * data;
	int dispositivi;
	Timestamps RiempiTimestamp(Timestamps, string, string);
	Movimenti RiempiMovimenti(Movimenti,string, string);
	void CompletaPacchetto(string mac, int timestamp);
	vector<Pacchetto_c> pacchetti;
	void svuotaVector();
};
#endif