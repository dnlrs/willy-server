
#include "DBTestUnit.h"
#include "stdafx.h"
#include "Database.h"
#include "packet.h"
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using namespace std;

int test_do(Database db)
{
    uint64_t anchors_mac[] = {
        69999999999990,
        69999999999992,
        69999999999994,
        69999999999996
    };

    vector<PACKET_T> test_v;
    int i, rs;
    int packets_nr = 10;
    vector<DB_PACKET_T> found_packets;

    for (i = 0; i < packets_nr; i++) {
        test_v.push_back(test_get_packet());
    }

    // add packets to db
    for (i = 0; i < packets_nr; i++) {

        cout << "Adding: ";
        test_print_packet(test_v[i]);
        if (db.add_packet(test_v[i], anchors_mac[rand() % 4])) {
            cout << "ERROR: " << db.get_error_msg() << endl;
        }

        found_packets = db.get_device_packets(test_v[i].mac_addr.compacted_mac, test_v[i].timestamp - 1, test_v[i].timestamp + 1);
        if (found_packets.size() > 0) {
            cout << "Added : ";
            test_print_packet(found_packets[0]);
        }
        else {
            cout << "ERROR adding packet" << endl;
        }
    }

    // add devices to db
    for (i = 0; i < packets_nr; i++) {
        DB_DEVICE_T device = test_get_device();
        db.add_device(device);
    }


    rs = db.cleanup_packets_table();
    if (rs) {
        cout << "ERROR: " << db.get_error_msg() << endl;
    }

    // get devices number in interval
    int devices_nr = db.get_devices_nr(1415463675, 1415463695);
    if (devices_nr >= 0) {
        cout << "Total number of devices seen: " << devices_nr << endl;
    }
    else {
        cout << "ERROR: " << db.get_error_msg() << endl;
    }

    // get number of persistent devices in interval
    devices_nr = db.get_persistent_devices(1415463680, 1415463683);
    if (devices_nr >= 0) {
        cout << "Persistent devices seen: " << devices_nr << endl;
    }
    else {
        cout << "ERROR: " << db.get_error_msg() << endl;
    }


    // get posistions in each interval for each device
    map<uint64_t, vector<DB_DEVICE_T>> positions = db.get_positions(1415463675, 1415463695);

    for (auto& position : positions) {
        cout << position.first << ":" << endl;
        for (int j = 0; j < (int)position.second.size(); j++) {
            cout << "    " << test_string_device(position.second[j]) << endl;
        }
    }


    // get presence timestamps for each device
    map<uint64_t, vector<uint64_t>> presence = db.get_presence_timestamps(1415463675, 1415463695);

    for (auto & entry : presence) {
        mac_t device_mac;
        device_mac.compacted_mac = entry.first;
        cout << mactos(device_mac) << ":" << endl;

        for (int j = 0; j < (int)entry.second.size(); j++) {
            cout << "    " << to_string(entry.second[j]) << endl;
        }
    }

    cout << "Test succeeded" << endl;

    return 0;

}




// returns a packet
PACKET_T test_get_packet()
{
    PACKET_T pack;


    pack.hash = test_get_hash();
    pack.ssid = "";
    pack.rssi = 40 + (rand() % 100);
    pack.mac_addr.compacted_mac = test_get_device_mac();
    pack.channel = 13;
    pack.timestamp = test_get_timestamp();
    pack.sequence_ctrl = 1 + (rand() % 100);

    return pack;
}


DB_DEVICE_T test_get_device() {

    DB_DEVICE_T device;

    device.mac = test_get_device_mac();
    device.timestamp = test_get_timestamp();
    device.pos_x = 0 + (rand() % 10);
    device.pos_y = 0 + (rand() % 10);

    return device;
}

uint64_t test_get_device_mac() {

    uint64_t devices_mac[] = {
        53526641582508,
        53526641582510,
        53526641582512,
        53526641582514
    };

    return devices_mac[rand() % 3];
}

uint64_t test_get_timestamp() {

    uint64_t ts_start = 1415463675;
    int interval = 20;

    return ts_start + (rand() % interval);
}


// returns a random hash
string test_get_hash()
{
    int len = 10;
    string hashes[] = {
        "frAQBc8Wsa1xVPfvJcrgRYwTiizs2trQ",
        "Blax3CF3EDNhm3soLBPh71YexuieaoEi",
        "4a2dREbbSqWy6yhKIDCdJOyapnxrpMCA",
        "dGc81tBDKsMlaZTXC1O8YFOGKjxRrJBd",
        "foaMeAjSWfchoZYFYZ5B6kzMCk8R6BEu",
        "cI6NX8DYdD3ojxSnqPTGfRyilOYGxlSX",
        "oH8S4kwIgTxSl1C00GOzOLMrbAyfKUUT",
        "blsaqv6UpdvNIsNrmwUlN5u9t3tgj2tu",
        "FkrFCJCmZFOv1QDIIXJNZI95hFQr77BI",
        "4Aj6PKnZpzRiKYpZgnSOKlq8AzrHqDoG"
    };
    return hashes[rand() % (len - 1)];
}



void test_print_packet(DB_PACKET_T pack)
{
    mac_t anchor_mac;
    anchor_mac.compacted_mac = pack.anchor_mac;
    mac_t device_mac;
    device_mac.compacted_mac = pack.mac;

    cout << "BOARD: " << mactos(anchor_mac)
        << "|| HASH: " << pack.hash
        << ", RSSI: " << pack.rssi
        << ", MAC: " << mactos(device_mac)
        << ", TIMESTAMP: " << pack.timestamp
        << ", SEQUENCE: " << pack.seq_number << endl;
}

void test_print_packet(PACKET_T pack)
{
    cout << "BOARD: " << "--:--:--:--:--:--"
        << "|| HASH: " << pack.hash
        << ", RSSI: " << pack.rssi
        << ", MAC: " << mactos(pack.mac_addr)
        << ", TIMESTAMP: " << pack.timestamp
        << ", SEQUENCE: " << pack.sequence_ctrl
        << ", CHANNEL: " << to_string(pack.channel)
        << ", SSID: " << pack.ssid << endl;
}

string test_string_device(DB_DEVICE_T device)
{
    mac_t device_mac;

    device_mac.compacted_mac = device.mac;

    return "" + mactos(device_mac) + " " + to_string(device.timestamp) + " " + to_string(device.pos_x) + " " + to_string(device.pos_y);
}

string test_random_hash() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int len = 32;
    string result;

    for (int i = 0; i < len; ++i) {
        result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    result[len] = 0;

    return result;
}