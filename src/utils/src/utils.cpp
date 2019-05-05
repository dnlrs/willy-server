#include "logger.h"
#include "utils.h"
#include <string>
#include <vector>
#include <algorithm>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <windows.h> // windef.h - do not include directly

std::string timetos(struct tm time)
{
    std::string rval;
    std::vector<std::string> days{
        "Sunday", "Monday", "Tuesday","Wednesday",
        "Thursday", "Friday","Saturday"
    };
    std::vector<std::string> months{
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    };

    rval += days[time.tm_wday] + ", " + months[time.tm_mon] + " "
        + std::to_string(time.tm_mday) + ", "
        + std::to_string(1900 + time.tm_year) + " "
        + std::to_string(time.tm_hour) + ":"
        + std::to_string(time.tm_min) + ":"
        + std::to_string(time.tm_sec);

    return rval;
}

struct tm epochTotm(const time_t rawtime)
{
    struct tm ptm;
    localtime_s(&ptm, &rawtime);
    return ptm;
}

uint64_t get_current_time()
{
    std::time_t current_time = std::time(nullptr);

    /* this may break since time_t type is not standard-specified, 
     * yet it is implemented as uint64_t in windows 
     */
    return (uint64_t)current_time;
}



constexpr size_t af_inet_address_size = 16;

int
str_is_valid_int(const char* str)
{
    if (!str) return 0;
    while (*str) {
        if (!isdigit(*str))
            return 0;
        ++str;
    }

    return 1;
}

int
str_is_valid_double(const char* str)
{
    if (!str) return 0;

    int v = 0;
    while (*str) {
        if (!isdigit(*str))
            if (*str == ',' || *str == '.')
                v++;
            else
                return 0;
        ++str;
    }

    return (v == 1);
}

std::string
wsa_etos(int error)
{
    char rval[default_error_str_length];
    memset(rval, 0, default_error_str_length);

    int check = 0;

    check =
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            0,
            rval,
            default_error_str_length,
            NULL);

    if (check == 0)
        return std::to_string(error);

    rval[check] = '\0';
    return std::string(rval);
}