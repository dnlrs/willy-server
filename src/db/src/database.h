#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED
#pragma once

#include "packet.h"
#include "device.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>

namespace db {
    
    /*
     * Notation:
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
    class database
    {
    public:

        database(std::string dbName, int anchorsNr) :
            db(nullptr), db_name(dbName), db_anchors_nr(anchorsNr) {};

        /* Close connection to database if open */
        ~database() 
        { 
            if (db != nullptr) 
                sqlite3_close(db); 
        }


        /* Sets the number of available anchors */
        inline void set_anchors_number(int anchorsNr) { db_anchors_nr = anchorsNr; }
    
        /*
         * Opens a connection towards the database.
         * 
         * If the database did not exist it is created.
         * If the tables did not exist they are created.
         * If tables did already exist: 
         *  - if reset is true they are dropped and re-created
         *  - if reset is false the database is not modified
         */
        void open(bool reset = false);

        /* Safely closes the database connection */
        void quit();


        /* Inserts a "raw" packet into the database. */
        void add_packet(PACKET_T packet, uint64_t anchor_mac);
    

        /* Inserts a device and its associated position into the database. */
        void add_device(device device_in);


        /* [deprecated]
         * Cleans the table with all packets received.
         * 
         * Removes from the received packets dataset all packets that have not
         * been received by all the anchors.
         */
        void cleanup_packets_table();


        /* [deprecated]
         * Deletes all packets before the timestamp passed as parameter.
         */
        void delete_packets_before(uint64_t timestamp);


        /*
         * Returns all packets belonging to a given device received in a given interval.
         * 
         * \param mac the mac of the device
         * \param ts_start the start of the interval
         * \param ts_end   the end of the interval
         * \return a vector with all packets found
         * \return an empty vector if no packets found or an error occurred, if
         *         an error occurred db_errmsg is set
         */
        std::vector<PACKET_T> 
            get_device_packets(uint64_t mac, uint64_t ts_start, uint64_t ts_end);
    

        /*
         * Returns the number of distinct devices found in a given interval.
         * 
         * The device is identified by its mac address.
         * 
         * \param ts_start the start of the interval
         * \param ts_end   the end of the interval
         * \return the number of distinct devices found
         * \return DB_ERROR if an error occurs (db_errmsg is set)
         */
        int get_devices_nr(uint64_t ts_start, uint64_t ts_end);


        /*
         * Returns the number of distinct always-present devices in a given interval.
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


        /* 
         * Returns all the devices along with their prosition for each sub-interval in 
         * the given interval
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
        std::map<uint64_t, std::vector<device>> 
            get_positions(uint64_t ts_start, uint64_t ts_end);

        /* 
         * Returns for each device all the timestamps when it has been seen in a given
         * interval.
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
        std::map<uint64_t, std::vector<uint64_t>> 
            get_presence_timestamps(uint64_t ts_start, uint64_t ts_end);

    private:
        sqlite3 *db;
        std::string   db_name;
        int      db_anchors_nr;

        std::map<uint64_t, std::map<uint64_t, int>> tmp_map_for_loc;
    };
}
#endif // DATABASE_H_INCLUDED