#pragma once
#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "packet.h"
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#define DB_ERROR -1

using namespace std;

/*!
 * TODO: these structures should be unform through the whole application
 */
typedef struct
{
    uint64_t mac;
    uint64_t timestamp;
    double pos_x;
    double pos_y;
} DB_DEVICE_T;

typedef struct
{
    string hash;
    int rssi;
    uint64_t mac;
    uint32_t seq_number;
    uint64_t timestamp;
    uint64_t anchor_mac;
} DB_PACKET_T;


/*!
 * notation:
 *  *    nullable attribute
 *  $..$ part of the primary key
 * 
 * Current tables in the database:
 *    packets($hash$, ssid*, rssi, $mac$, channel*, seq_number*, timestamp, anchor_mac)
 *    devices($mac$, $timestamp$, pos_x, pos_y)
 * 
 * Error handling:
 * This class has an interval variable that keeps track of the last error occurred in
 * a string format. So if a function returns an error you may access this variable to
 * know what happened. The variable is accessed through `get_error_msg()` function.
 * 
 * Generally the functions that return an integer will return DB_ERROR if an error
 * occurs while the functions that return a vector of a map will return an empty
 * vector of an empty map. In either cases the internal string error variable will 
 * be set. 
 */
class Database
{
public:

    Database();
    Database(string dbName, int anchorsNr);
    ~Database();

    /*! Sets the number of available anchors
     */
    void set_anchors_number(int anchorsNr);
    
    /*! Initialize the database connection and necessary tables.
     * 
     * Currently drops all tables and recreates them. 
     * This may have to be changed or alternatively this function will have to
     * be called only once if we want to keep data available between different
     * restarts of the application
     * 
     * \return SQLITE_OK if OK
     * \return DB_ERROR  if an error occurs (db_errmsg is set)
     */
    int init();

    /*! Safely closes the database connection 
     */
    void quit();


    /*! Logs a "raw" packet into the database.
     * 
     * 
     * \param packet the packet to log
     * \return SQLITE_OK if OK
     * \return DB_ERROR if an error occurs (db_errmsg is set)
     */
    int add_packet(PACKET_T packet, uint64_t anchor_mac);
    

    /*! Logs a device nad its associated position into the database.
     * 
     * 
     * \param device the device to be logged
     * \return SQLITE_OK if OK
     * \return DB_ERROR if an error occurs (db_errmsg is set)
     */
    int add_device(DB_DEVICE_T device);


    /*! Cleans the table with all packets received.
     * 
     * Removes from the received packets dataset all packets that have not
     * been received by all the anchors.
     * 
     * \return SQLITE_OK if OK
     * \return DB_ERROR if an error occurs (db_errmsg is set)
     */
    int cleanup_packets_table();


    /*! Deletes all packets before the timestamp passed as parameter.
     * 
     * \param timestamp 
     * \return SQLITE_OK if OK
     * \return DB_ERROR if and erro occurs (db_errmsg is set) 
     */
    int delete_packets_before(uint64_t timestamp);


    /*! Returns all packets belonging to a given device received in a given interval.
     * 
     * \param mac the mac of the device
     * \param ts_start the start of the interval
     * \param ts_end   the end of the interval
     * \return a vector with all packets found
     * \return an empty vector if no packets found or an error occurred, if
     *         an error occurred db_errmsg is set
     */
    vector<DB_PACKET_T> 
        get_device_packets(uint64_t mac, uint64_t ts_start, uint64_t ts_end);
    

    /*! Returns the number of distinct devices found in a given interval.
     * 
     * The device is identified by its mac address.
     * 
     * \param ts_start the start of the interval
     * \param ts_end   the end of the interval
     * \return the number of distinct devices found
     * \return DB_ERROR if an error occurs (db_errmsg is set)
     */
    int get_devices_nr(uint64_t ts_start, uint64_t ts_end);


    /*! Returns the number of distinct always-present devices in a given interval.
     * 
     * This function calculates the number of different timestamps found in the
     * database in the given interval and selects devices present in all the
     * timestamps found. 
     * 
     * However this implementation may not work because it depends on how data is
     * stored in the database (unit of timestamp?) and how often devices send probe
     * requests (every sec, every 10 secs, every minute?)
     * 
     * \param ts_start the start of the interval
     * \param ts_end   the end of the interval
     * \return the number of always-present devices in the given interval
     * \return DB_ERROR if an error occurs (db_errmsg is set)
     */
    int get_persistent_devices(uint64_t ts_start, uint64_t ts_end);


    /*! Returns all the devices along with their prosition for each sub-interval in 
     *  the given interval
     * 
     * Useful for movement visualization. 
     * 
     * However this implementation may not work because it depends on how data is
     * stored in the database (unit of timestamp?) and how often devices send probe
     * requests (every sec, every 10 secs, every minute?)
     * 
     * return logical format:
     *  [timestamp, [device1, device2, device3...]]
     * 
     * \param ts_start the start of the interval
     * \param ts_end the end of the interval
     * \return a map with the list of devices for each timestamp
     * \return and empty map if no devices found or an error occurred, in this last
     *         case db_errmsg is set 
     */
    map<uint64_t, vector<DB_DEVICE_T>> 
        get_positions(uint64_t ts_start, uint64_t ts_end);

    /*! Returns for each device all the timestamps when it has been seen in a given
     *  interval.
     * 
     * However this implementation may not work because it depends on how data is
     * stored in the database (unit of timestamp?) and how often devices send probe
     * requests (every sec, every 10 secs, every minute?)
     * 
     * return logical format:
     *  [device_mac, [timestamp1, timestamp2, timestamp3...]]
     * 
     * \param ts_start the start of the interval
     * \param ts_end   the end of the interval
     * \return a map with the list of intervals for each device
     * \return an empty map if no devices found in given interval or an error 
     *         occurred, in this last case db_errmsg is set
     */
    map<uint64_t, vector<uint64_t>> 
        get_presence_timestamps(uint64_t ts_start, uint64_t ts_end);

    /*! Returns a string message about the last error occurred.
     * 
     * \return a string with the last erroroccurred
     */
    string get_error_msg() { return db_errmsg; }

private:
    sqlite3 * db;
    string   db_name;
    int      db_anchors_nr;
    string   db_errmsg;
};
#endif