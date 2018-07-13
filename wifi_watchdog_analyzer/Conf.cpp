#include "stdafx.h"
#include "Conf.h"


Conf::Conf(const char* filename)
{
	/* open file & read receiver num */
	this->finished = false;
	file.open(filename, ios::in | ios::binary);
	if (file.fail())
		throw Conf_exception("No such configuration file");
	file >> this->receiver_n;
	this->next_line();
}

void Conf::operator() ()
{
	if (receiver_n == 0)
		throw Conf_exception("No configurations are loaded");
	if (actual_recv == receiver_n || this->finished)
		throw Conf_exception("Configurations info are finished");
	if(file.fail())
		throw Conf_exception("No such configuration file");

	/*  read the next receiver */
	string s;
	getline(file, s, ' ');
	for (size_t i = 0, j = 0; i < s.size() && j < MAC_BYTES; i += 3, j++)
		this->mac_addr.raw_mac[j] = std::stoi(s.substr(i, 2), NULL, 16);
	file >> this->curr_x;
	file >> this->curr_y;
	//debugging prints
	cout << mactos(this->mac_addr) << " " << this->curr_x << " " << this->curr_y << endl;

	this->actual_recv++;
	this->next_line();   // in case of eof it stops to extracting
	if (this->actual_recv == this->receiver_n)
		file.close();
}

void Conf::next_line()
{
	char c;
	do {
		c = file.peek();
		file.ignore(1);
	} while (c != EOF && c != '\n');
	if (c == EOF)
		this->finished = true;
}

Conf::~Conf()
{	
	file.close();
}
