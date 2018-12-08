#include "stdafx.h"
#include "Database.h"
#include "db_exception.h"
#include "packet.h"
#include <iostream>
#include <cstdint> /* for uint64_t */
#include <fstream>
#include <string>
#include <map>
#include <iterator>

using namespace std;

Database::Database()
{
    db = nullptr;
    db_name.clear();
    db_name = string("database.db");
    db_anchors_nr = 0;
}


Database::Database(string dbName, int anchorsNr)
{
    db = nullptr;
    db_name.clear();
    this->db_name = dbName;
    this->db_anchors_nr = anchorsNr;
}


Database::~Database()
{
    if (db != nullptr)
        sqlite3_close(db);
}


void 
Database::set_anchors_number(int anchorsNr)
{
    this->db_anchors_nr = anchorsNr;
}


/*
Initialize the database connection and necessary tables.
*/
void
Database::init() {

    string db_errmsg;
    
    /* open db connection */
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(db);
        sqlite3_close(db); 
        db = nullptr;
        throw db_exception(db_errmsg.c_str());
    }

    /* create tables if don't already exist */
    string sql = "DROP TABLE IF EXISTS packets;         \
                  DROP TABLE IF EXISTS devices;         \
                  CREATE TABLE IF NOT EXISTS packets (  \
                        hash        TEXT NOT NULL,      \
                        ssid        TEXT,               \
                        rssi        INTEGER NOT NULL,   \
                        mac         INTEGER NOT NULL,   \
                        channel     INTEGER,            \
                        seq_number  INTEGER,            \
                        timestamp   INTEGER NOT NULL,   \
                        anchor_mac  INTEGER NOT NULL,   \
                        PRIMARY KEY (hash, mac));       \
                  CREATE TABLE IF NOT EXISTS devices (  \
                        mac         INTEGER NOT NULL,   \
                        timestamp   INTEGER NOT NULL,   \
                        pos_x       REAL NOT NULL,      \
                        pos_y       REAL NOT NULL,      \
                        PRIMARY KEY (mac, timestamp));";

    char* errmsg;
    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg) != SQLITE_OK) {
        string db_errmsg = string(errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        db = nullptr;
        throw db_exception(db_errmsg.c_str());
    }
}

/*
Safely close the database connection
*/
void 
Database::quit()
{
    sqlite3_close(db);
    db = NULL;
}


void
Database::add_packet(PACKET_T packet, uint64_t anchor_mac)
{
    string db_errmsg;

    if (db == nullptr)
        throw db_exception("no db connection"); 

    string   hash = packet.hash;
    string   ssid = packet.ssid.empty() ? "NULL" : packet.ssid;
    int      rssi = packet.rssi;
    uint64_t mac = (uint64_t)packet.mac_addr.compacted_mac;
    int      channel = packet.channel;
    int      seq_number = packet.sequence_ctrl;
    uint64_t timestamp = packet.timestamp;

    string sql = "INSERT INTO packets(hash, ssid, rssi, mac, channel, seq_number, timestamp, anchor_mac) \
                  VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int result;

    db_errmsg.clear();

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_text(stmt, 1, hash.c_str(), hash.length(), SQLITE_STATIC) ||
        sqlite3_bind_int(stmt, 3, rssi) ||
        sqlite3_bind_int64(stmt, 4, mac) ||
        sqlite3_bind_int(stmt, 5, channel) ||
        sqlite3_bind_int(stmt, 6, seq_number) ||
        sqlite3_bind_int64(stmt, 7, timestamp) ||
        sqlite3_bind_int64(stmt, 8, anchor_mac)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    if (!packet.ssid.empty()) {
        if (sqlite3_bind_text(stmt, 2, ssid.c_str(), ssid.length(), SQLITE_STATIC)) {
            db_errmsg = string(sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            throw db_exception(db_errmsg.c_str());
        }
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);

    // ---------- TO BE MOVED -------------------------------------------
    // manage localization process start
    auto device      = tmp_map_for_loc[mac];
    auto anchor_rssi = device[anchor_mac];
    
    if (anchor_rssi != 0)
        throw db_exception("anchor rssi already inserted");

    anchor_rssi = rssi;

    if (device.size() == db_anchors_nr) {
        // add localization call 
        // add db DEVICE insertion
    }
}

void
Database::add_device(DB_DEVICE_T device)
{

    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    string db_errmsg;
    string sql = "INSERT INTO devices(mac, timestamp, pos_x, pos_y) \
                  VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int result;

    db_errmsg.clear();

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, device.mac) ||
        sqlite3_bind_int64(stmt, 2, device.timestamp) ||
        sqlite3_bind_double(stmt, 3, device.pos_x) ||
        sqlite3_bind_double(stmt, 4, device.pos_y)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);
}

void
Database::cleanup_packets_table()
{
    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    string db_errmsg;
    string sql = "DELETE FROM packets \
                  WHERE hash IN (SELECT hash \
                                 FROM packets \
                                 GROUP BY hash \
                                 HAVING COUNT(DISTINCT anchor_mac) < ?);";

    sqlite3_stmt *stmt = NULL;
    int result;


    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int(stmt, 1, db_anchors_nr)) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);
}

void
Database::delete_packets_before(uint64_t timestamp) {

    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    string db_errmsg;
    string sql = "DELETE FROM packets \
                  WHERE timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int result;


    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, timestamp)) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);
}

vector<DB_PACKET_T>
Database::get_device_packets(uint64_t mac, uint64_t ts_start, uint64_t ts_end)
{
    string db_errmsg;

    if (ts_start > ts_end) {
        return vector<DB_PACKET_T>();
    }

    vector<DB_PACKET_T> result;

    string sql = "SELECT hash, rssi, mac, seq_number, timestamp, anchor_mac  \
                  FROM packets                                               \
                  WHERE mac = ? AND timestamp > ? AND timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;


    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        return vector<DB_PACKET_T>();
    }


    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, mac) ||
        sqlite3_bind_int64(stmt, 2, ts_start) ||
        sqlite3_bind_int64(stmt, 3, ts_end)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return vector<DB_PACKET_T>();
    }

    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {

        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        DB_PACKET_T packet;

        packet.hash = string((char*)sqlite3_column_text(stmt, 0));
        packet.rssi = sqlite3_column_int(stmt, 1);
        packet.mac = sqlite3_column_int64(stmt, 2);
        packet.seq_number = sqlite3_column_int(stmt, 3);
        packet.timestamp = sqlite3_column_int64(stmt, 4);
        packet.anchor_mac = sqlite3_column_int64(stmt, 5);

        result.push_back(packet);
        rs = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

int Database::get_devices_nr(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    int attempts;
    int result;

    string db_errmsg;
    string sql = "SELECT COUNT(DISTINCT mac) \
                  FROM packets               \
                  WHERE timestamp > ? AND timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }


    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    attempts = 10;
    rs = SQLITE_BUSY;
    while (rs == SQLITE_BUSY && attempts >= 0) {
        rs = sqlite3_step(stmt);
        attempts--;
    }

    switch (rs) {
    case SQLITE_ROW: // there is data
        result = sqlite3_column_int(stmt, 0);
        break;
    case SQLITE_DONE: // success but no data (0 devices)
        result = 0;
        break;
    default:
        db_errmsg = string(sqlite3_errmsg(db));
        result = -1;
        break;
    }

    sqlite3_finalize(stmt);
    return result;
}

int Database::get_persistent_devices(uint64_t ts_start, uint64_t ts_end)
{

    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    int result;

    string sql = "SELECT COUNT(DISTINCT mac)    \
                  FROM devices                  \
                  WHERE timestamp >= ? AND timestamp <= ? \
                  GROUP BY mac \
                  HAVING COUNT(DISTINCT timestamp) = (SELECT COUNT(DISTINCT timestamp) \
                                                      FROM devices \
                                                      WHERE timestamp >= ? AND timestamp <= ?);";

    sqlite3_stmt *stmt = NULL;
    int rs;

    string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end) ||
        sqlite3_bind_int64(stmt, 3, ts_start) ||
        sqlite3_bind_int64(stmt, 4, ts_end)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    int attempts = 10;
    rs = SQLITE_BUSY;
    while (rs == SQLITE_BUSY && attempts >= 0) {
        rs = sqlite3_step(stmt);
        attempts--;
    }

    switch (rs) {
    case SQLITE_ROW: // there is data
        result = sqlite3_column_int(stmt, 0);
        break;
    case SQLITE_DONE: // success but no data (0 devices)
        result = 0;
        break;
    default:
        db_errmsg = string(sqlite3_errmsg(db));
        result = -1;
        break;
    }

    sqlite3_finalize(stmt);
    return result;
}

map<uint64_t, vector<DB_DEVICE_T>>
Database::get_positions(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    map<uint64_t, vector<DB_DEVICE_T>> result;
    map<uint64_t, vector<DB_DEVICE_T>>::iterator it;

    string sql = "SELECT mac, timestamp, pos_x, pos_y   \
                  FROM devices                          \
                  WHERE timestamp >= ? AND timestamp <= ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        return map<uint64_t, vector<DB_DEVICE_T>>();
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return map<uint64_t, vector<DB_DEVICE_T>>();
    }

    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {

        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        DB_DEVICE_T device;

        device.mac = sqlite3_column_int64(stmt, 0);
        device.timestamp = sqlite3_column_int64(stmt, 1);
        device.pos_x = sqlite3_column_double(stmt, 2);
        device.pos_y = sqlite3_column_double(stmt, 3);

        it = result.find(device.timestamp);
        if (it == result.end()) {
            vector<DB_DEVICE_T> tmp_vector;
            tmp_vector.push_back(device);
            result.insert(make_pair(device.timestamp, tmp_vector));
        }
        else {
            it->second.push_back(device);
        }

        rs = sqlite3_step(stmt);
    }


    sqlite3_finalize(stmt);
    return result;
}


map<uint64_t, vector<uint64_t>>
Database::get_presence_timestamps(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    map<uint64_t, vector<uint64_t>> result;
    map<uint64_t, vector<uint64_t>>::iterator entry;

    string sql = "SELECT mac, timestamp                 \
                  FROM devices                          \
                  WHERE timestamp >= ? AND timestamp <= ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = string(sqlite3_errmsg(db));
        return map<uint64_t, vector<uint64_t>>();
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {

        db_errmsg = string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return map<uint64_t, vector<uint64_t>>();
    }


    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {

        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        uint64_t mac = sqlite3_column_int64(stmt, 0);
        uint64_t timestamp = sqlite3_column_int64(stmt, 1);

        entry = result.find(mac);
        if (entry == result.end()) {
            vector<uint64_t> tmp_vector;
            tmp_vector.push_back(timestamp);
            result.insert(make_pair(mac, tmp_vector));
        }
        else {
            entry->second.push_back(timestamp);
        }

        rs = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}