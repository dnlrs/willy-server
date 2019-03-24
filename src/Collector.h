#pragma once

#include <mutex>
#include <map>
#include "packet.h"




class Collector
{
public:
	Collector(int board_n) : board_n(board_n){};
	~Collector() {};

	bool insertNewPacket(PACKET_T pack, uint64_t board_mac) 
	{
		std::lock_guard<std::mutex> lg(this->mtx);
		this->packbuff[pack.hash][board_mac] = pack.rssi;
		this->timebuff[pack.hash] = pack.timestamp;
		

		if (this->packbuff[pack.hash].size() == this->board_n)
			return true;
		return false;
	}

	std::map<uint64_t, int32_t> returnPacketInfo(std::string hash, uint32_t& time)
	{
		std::lock_guard<std::mutex> lg(this->mtx);

		std::map<uint64_t, int32_t> currpack = this->packbuff[hash];
		time = this->timebuff[hash];
		
		/* remove elements */
		this->packbuff.erase(hash);
		this->timebuff.erase(hash);

		return currpack;
	}

	/* TODO refreshing old items */



private:
	int board_n;
	mutex mtx;
	std::map<std::string, std::map<uint64_t, int32_t>> packbuff;
	std::map<std::string, uint32_t> timebuff;
};

