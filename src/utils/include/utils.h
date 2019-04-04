#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#pragma once

#include <string>
#include <time.h>

constexpr int default_error_str_length = 1024; // for WSA error string formatting


/* Converts a struct tm to string (Monday, January 01, 1900 13:00:00) */
std::string timetos(struct tm time);

/* Converts epoch time to local time */
struct tm epochTotm(const time_t rawtime);



/* Converts a mac from string to byte format (host byte order) */
uint64_t mac_str2int(const std::string mac);

/* Converts a mac from byte format (host byte order) to string */
std::string mac_int2str(const uint64_t mac);

/* Checks if a mac in string format is valid */
int mac_is_valid(const char* mac);



/* Formats WSA error to string */
std::string wsa_etos(int error);


#endif // !UTILS_H_INCLUDED
