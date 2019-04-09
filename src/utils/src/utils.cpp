#include "utils.h"
#include <string>
#include <vector>
#include <algorithm>
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




uint64_t mac_str2int(const std::string mac)
{
    if (!mac_is_valid(mac.c_str()))
        return 0;

    std::string local_mac(mac);
    local_mac.erase(
        remove(local_mac.begin(), local_mac.end(), ':'),
        local_mac.end());

    local_mac.erase(
        remove(local_mac.begin(), local_mac.end(), '-'),
        local_mac.end());

    return strtoull(local_mac.c_str(), NULL, 16);
}

std::string mac_int2str(const uint64_t mac)
{
    static const char* digits = "0123456789ABCDEF";
    uint64_t local_mac = mac;
    char rval[18]; // xx:xx:xx:xx:xx:xx'\0'

    int pos = 17;
    for (int byte = 5; byte >= 0; byte--) {
        rval[pos--] = ':';
        rval[pos--] = digits[local_mac & 0x0f];
        local_mac >>= 4;
        rval[pos--] = digits[local_mac & 0x0f];
        local_mac >>= 4;
    }
    rval[17] = '\0';

    return std::string(rval);
}

int mac_is_valid(const char* mac)
{
    if (!mac)
        return 0;

    int i = 0;
    int s = 0;

    while (*mac) {
        if (isxdigit(*mac))
            i++;
        else if (*mac == ':' || *mac == '-') {
            if (i == 0 || i / 2 - 1 != s)
                break;
            ++s;
        }
        else
            s = -1;
        ++mac;
    }
    return (i == 12 && (s == 5 || s == 0));
}

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