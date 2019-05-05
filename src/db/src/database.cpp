#include "database.h"
#include "db_exception.h"
#include "packet.h"
#include <iostream>
#include <cstdint> /* for uint64_t */
#include <fstream>
#include <string>
#include <map>
#include <iterator>

void
db::database::open(bool reset) {
    /* open db connection */
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(db);
        sqlite3_close(db);
        db = nullptr;
        throw db_exception(errmsg);
    }

    std::string sql = "";

    if (reset)
        sql.append("DROP TABLE IF EXISTS packets; \
                     DROP TABLE IF EXISTS devices;");

    /* create tables if don't already exist */
    sql.append("CREATE TABLE IF NOT EXISTS packets (   \
                    hash        TEXT NOT NULL,          \
                    ssid        TEXT,                   \
                    rssi        INTEGER NOT NULL,       \
                    mac         INTEGER NOT NULL,       \
                    channel     INTEGER,                \
                    seq_number  INTEGER,                \
                    timestamp   INTEGER NOT NULL,       \
                    anchor_mac  INTEGER NOT NULL,       \
                    PRIMARY KEY (hash, mac, anchor_mac));   \
                CREATE TABLE IF NOT EXISTS devices (    \
                    mac         INTEGER NOT NULL,       \
                    timestamp   INTEGER NOT NULL,       \
                    pos_x       REAL NOT NULL,          \
                    pos_y       REAL NOT NULL,          \
                    PRIMARY KEY (mac, timestamp));");

    char* errmsg;
    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg) != SQLITE_OK) {
        std::string db_errmsg = std::string(errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        db = nullptr;
        throw db_exception(db_errmsg.c_str());
    }
}

void
db::database::quit()
{
    sqlite3_close(db); // if db == NULL then harmless no-op
    db = nullptr;
}

void
db::database::add_packet(packet new_packet, mac_addr anchor_mac)
{
    std::string db_errmsg;

    if (db == nullptr)
        throw db_exception("no db connection");

    std::string   hash = new_packet.hash;
    std::string   ssid = new_packet.ssid.empty() ? "NULL" : new_packet.ssid;
    int           rssi = new_packet.rssi;
    mac_addr      mac  = new_packet.device_mac;
    int           channel    = new_packet.channel;
    int           seq_number = new_packet.sequence_ctrl;
    uint64_t      timestamp  = new_packet.timestamp;

    std::string sql = "INSERT INTO packets(hash, ssid, rssi, mac, channel, seq_number, timestamp, anchor_mac) \
                       VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int result;

    db_errmsg.clear();

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_text(stmt, 1, hash.c_str(), (int)hash.length(), SQLITE_STATIC) ||
        sqlite3_bind_int(stmt, 3, rssi) ||
        sqlite3_bind_int64(stmt, 4, mac.get()) ||
        sqlite3_bind_int(stmt, 5, channel) ||
        sqlite3_bind_int(stmt, 6, seq_number) ||
        sqlite3_bind_int64(stmt, 7, timestamp) ||
        sqlite3_bind_int64(stmt, 8, anchor_mac.get())) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    if (!new_packet.ssid.empty()) {
        if (sqlite3_bind_text(stmt, 2, ssid.c_str(), (int)ssid.length(), SQLITE_STATIC)) {
            db_errmsg = std::string(sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            throw db_exception(db_errmsg.c_str());
        }
    }

    // execute statement
    int rcode = sqlite3_step(stmt);
    if (rcode != SQLITE_DONE) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        switch (rcode) {
        case SQLITE_CONSTRAINT:
            throw db_exception(
                db_errmsg.c_str(), db_exception::type::constraint_error);
            break;
        default:
            throw db_exception(db_errmsg.c_str());
        }
    }

    sqlite3_finalize(stmt);
}

void
db::database::add_device(device device_in)
{
    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    std::string db_errmsg;
    std::string sql = "INSERT INTO devices(mac, timestamp, pos_x, pos_y) \
                       VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int result;

    db_errmsg.clear();

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, device_in.mac.get()) ||
        sqlite3_bind_int64(stmt, 2, device_in.timestamp) ||
        sqlite3_bind_double(stmt, 3, device_in.position.x) ||
        sqlite3_bind_double(stmt, 4, device_in.position.y)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    int rcode = sqlite3_step(stmt);
    if (rcode != SQLITE_DONE) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        switch (rcode) {
        case SQLITE_CONSTRAINT:
            throw db_exception(
                db_errmsg.c_str(), db_exception::type::constraint_error);
            break;
        default:
            throw db_exception(db_errmsg.c_str());
        }
    }

    sqlite3_finalize(stmt);
}

void
db::database::cleanup_packets_table()
{
    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    std::string db_errmsg;
    std::string sql = "DELETE FROM packets \
                       WHERE hash IN (SELECT hash \
                                      FROM packets \
                                      GROUP BY hash \
                                      HAVING COUNT(DISTINCT anchor_mac) < ?);";

    sqlite3_stmt *stmt = NULL;
    int result;

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int(stmt, 1, db_anchors_nr)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);
}

void
db::database::delete_packets_before(uint64_t timestamp) {
    if (db == nullptr)
        throw db_exception("no db connection"); // no db connection

    std::string db_errmsg;
    std::string sql = "DELETE FROM packets \
                       WHERE timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int result;

    // prepare statement
    result = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (result != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, timestamp)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    // execute statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw db_exception(db_errmsg.c_str());
    }

    sqlite3_finalize(stmt);
}

std::vector<packet>
db::database::get_device_packets(mac_addr mac, uint64_t ts_start, uint64_t ts_end)
{
    std::string db_errmsg;

    if (ts_start > ts_end) {
        return std::vector<packet>();
    }

    std::vector<packet> result;

    std::string sql = "SELECT hash, rssi, mac, seq_number, timestamp, anchor_mac  \
                       FROM packets                                               \
                       WHERE mac = ? AND timestamp > ? AND timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        return std::vector<packet>();
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, mac.get()) ||
        sqlite3_bind_int64(stmt, 2, ts_start) ||
        sqlite3_bind_int64(stmt, 3, ts_end)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return std::vector<packet>();
    }

    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {
        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        packet new_packet;

        new_packet.hash          = std::string((char*)sqlite3_column_text(stmt, 0));
        new_packet.rssi          = sqlite3_column_int(stmt, 1);
        new_packet.device_mac    = sqlite3_column_int64(stmt, 2);
        new_packet.sequence_ctrl = sqlite3_column_int(stmt, 3);
        new_packet.timestamp     = sqlite3_column_int64(stmt, 4);
        new_packet.anchor_mac    = sqlite3_column_int64(stmt, 5);

        result.push_back(new_packet);
        rs = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

int db::database::get_devices_nr(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    int attempts;
    int result;

    std::string db_errmsg;
    std::string sql = "SELECT COUNT(DISTINCT mac) \
                       FROM packets               \
                       WHERE timestamp > ? AND timestamp < ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
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
        db_errmsg = std::string(sqlite3_errmsg(db));
        result = -1;
        break;
    }

    sqlite3_finalize(stmt);
    return result;
}

int
db::database::get_persistent_devices(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    int result;

    std::string sql = "SELECT COUNT(DISTINCT mac)    \
                       FROM devices                  \
                       WHERE timestamp >= ? AND timestamp <= ? \
                       GROUP BY mac \
                       HAVING COUNT(DISTINCT timestamp) = (SELECT COUNT(DISTINCT timestamp) \
                                                           FROM devices \
                                                           WHERE timestamp >= ? AND timestamp <= ?);";

    sqlite3_stmt *stmt = NULL;
    int rs;

    std::string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        throw db_exception(db_errmsg.c_str());
    }

    // bind parameterssz
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end) ||
        sqlite3_bind_int64(stmt, 3, ts_start) ||
        sqlite3_bind_int64(stmt, 4, ts_end)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
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
        db_errmsg = std::string(sqlite3_errmsg(db));
        result = -1;
        break;
    }

    sqlite3_finalize(stmt);
    return result;
}

std::map<uint64_t, std::vector<device>>
db::database::get_positions(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    std::map<uint64_t, std::vector<device>> result;
    std::map<uint64_t, std::vector<device>>::iterator it;

    std::string sql = "SELECT mac, timestamp, pos_x, pos_y   \
                       FROM devices                          \
                       WHERE timestamp >= ? AND timestamp <= ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    std::string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        return std::map<uint64_t, std::vector<device>>();
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return std::map<uint64_t, std::vector<device>>();
    }

    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {
        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        device rdevice;

        rdevice.mac        = sqlite3_column_int64(stmt, 0);
        rdevice.timestamp  = sqlite3_column_int64(stmt, 1);
        rdevice.position.x = sqlite3_column_double(stmt, 2);
        rdevice.position.y = sqlite3_column_double(stmt, 3);

        it = result.find(rdevice.timestamp);
        if (it == result.end()) {
            std::vector<device> tmp_vector;
            tmp_vector.push_back(rdevice);
            result.insert(make_pair(rdevice.timestamp, tmp_vector));
        }
        else {
            it->second.push_back(rdevice);
        }

        rs = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::map<mac_addr, std::vector<uint64_t>>
db::database::get_presence_timestamps(uint64_t ts_start, uint64_t ts_end)
{
    if (ts_start > ts_end)
        throw db_exception("start timestamp grater than end timestamp");

    std::map<mac_addr, std::vector<uint64_t>> result;
    std::map<mac_addr, std::vector<uint64_t>>::iterator entry;

    std::string sql = "SELECT mac, timestamp                 \
                       FROM devices                          \
                       WHERE timestamp >= ? AND timestamp <= ?;";

    sqlite3_stmt *stmt = NULL;
    int rs;

    std::string db_errmsg;

    // prepare statement
    rs = sqlite3_prepare_v2(db, sql.c_str(), (int)sql.length(), &stmt, NULL);
    if (rs != SQLITE_OK || stmt == NULL) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        return std::map<mac_addr, std::vector<uint64_t>>();
    }

    // bind parameters
    if (sqlite3_bind_int64(stmt, 1, ts_start) ||
        sqlite3_bind_int64(stmt, 2, ts_end)) {
        db_errmsg = std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return std::map<mac_addr, std::vector<uint64_t>>();
    }

    rs = sqlite3_step(stmt);
    while (rs != SQLITE_DONE) {
        if (rs == SQLITE_BUSY)
            continue;
        if (rs == SQLITE_ERROR || rs == SQLITE_MISUSE)
            break;

        mac_addr mac;
        uint64_t timestamp = 0;
        mac = (uint64_t)sqlite3_column_int64(stmt, 0);
        timestamp = sqlite3_column_int64(stmt, 1);

        entry = result.find(mac);
        if (entry == result.end()) {
            std::vector<uint64_t> tmp_vector;
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