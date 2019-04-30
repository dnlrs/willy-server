#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#pragma once

#include <string>
#include <ctime>

constexpr int default_error_str_length = 1024; // for WSA error string formatting


/* Converts a struct tm to string (Monday, January 01, 1900 13:00:00) */
std::string timetos(struct tm time);

/* Converts epoch time to local time */
struct tm epochTotm(const time_t rawtime);

/* returns the current time of the system as time since epoch */
uint64_t get_current_time();

// string format checking
int str_is_valid_int(const char* str);
int str_is_valid_double(const char* str);

/* Formats WSA error to string */
std::string wsa_etos(int error);


#endif // !UTILS_H_INCLUDED
