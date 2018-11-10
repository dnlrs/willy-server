#pragma once

#ifndef _DB_TEST_UNIT_
#define _DB_TEST_UNIT_


#include "Database.h"
#include "packet.h"


int test_do(Database db);
// returns a packet
PACKET_T test_get_packet();
// returns a device
DB_DEVICE_T test_get_device();
// returns a random hash
string test_get_hash();


void test_print_packet(DB_PACKET_T pack);
void test_print_packet(PACKET_T pack);
string test_string_device(DB_DEVICE_T device);

uint64_t test_get_device_mac();
uint64_t test_get_timestamp();

string test_random_hash();




#endif