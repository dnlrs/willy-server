#include "mac_addr.h"
#include "winsock2.h"

mac_addr::mac_addr()
{
    addr[0] = 0;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 0;
    addr[4] = 0;
    addr[5] = 0;
}

mac_addr::mac_addr(
    const std::string str_mac)
{
    if (!is_valid_mac_string(str_mac.c_str()))
        throw utl_exception("mac_addr ctor: invalid mac string");

    parse_mac_string(str_mac);
}

mac_addr::mac_addr(const mac_addr& other)
{
    addr[0] = other.addr[0];
    addr[1] = other.addr[1];
    addr[2] = other.addr[2];
    addr[3] = other.addr[3];
    addr[4] = other.addr[4];
    addr[5] = other.addr[5];
}

bool mac_addr::operator==(const mac_addr& other) const {
    return uint64() == other.uint64();
}
bool mac_addr::operator==(const std::string str_mac) const {
    return *this == mac_addr(str_mac);
}
bool mac_addr::operator==(uint64_t uint64_mac) const {
    return uint64() == uint64_mac;
}
bool mac_addr::operator<(const mac_addr& other) const {
    return uint64() < other.uint64();
}
bool mac_addr::operator<(const std::string str_mac) const {
    return *this < mac_addr(str_mac);
}
bool mac_addr::operator<(uint64_t uint64_mac) const {
    return uint64() < uint64_mac;
}

mac_addr& mac_addr::operator=(const std::string& str_mac)
{
    if (!is_valid_mac_string(str_mac.c_str()))
        throw utl_exception("mac_addr op=: invalid mac string");

    parse_mac_string(str_mac);

    return *this;
}

mac_addr& mac_addr::operator=(const uint64_t int_mac)
/*
 * 0x0000aabbccddeeff
 *
 * big endian (network byte order):
 * |00|00|aa|bb|cc|dd|ee|ff|
 *  -  -  0  1  2  3  4  5
 */
{
    uint64_t local_int_mac = int_mac;

    addr[5] = local_int_mac & 0x00ff;
    addr[4] = (local_int_mac >> 8) & 0x00ff;
    addr[3] = (local_int_mac >> 16) & 0x00ff;
    addr[2] = (local_int_mac >> 24) & 0x00ff;
    addr[1] = (local_int_mac >> 32) & 0x00ff;
    addr[0] = (local_int_mac >> 40) & 0x00ff;

    return *this;
}

uint8_t& mac_addr::operator[](int index)
{
    if (index < 0 || index >= mac_length)
        throw utl_exception("mac_addr: index out of bounds");

    return addr[index];
}

bool mac_addr::is_valid()
{
    return (uint64() != 0);
}

void mac_addr::clear()
{
    addr[0] = 0;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 0;
    addr[4] = 0;
    addr[5] = 0;
}

std::string mac_addr::str() const
/*
 * aa:bb:cc:dd:ee:ff
 *
 * big endian (network byte order):
 *     |aa|bb|cc|dd|ee|ff|
 *      0  1  2  3  4  5
 *
 * note: since the string is built backwards also
 * the byte array is read bacwards.
 */
{
    static const char* digits = "0123456789ABCDEF";
    char rval[18]; // xx:xx:xx:xx:xx:xx'\0'

    int pos = 17;
    for (int byte_index = (mac_length - 1); byte_index >= 0; byte_index--) {
        uint8_t current_byte = addr[byte_index];
        rval[pos--] = ':';
        rval[pos--] = digits[current_byte & 0x0f];
        current_byte >>= 4;
        rval[pos--] = digits[current_byte & 0x0f];
    }
    rval[17] = '\0';

    return std::string(rval);
}

uint64_t mac_addr::uint64() const
/*
 * 0x0000aabbccddeeff
 *
 * big endian (network byte order):
 *     |00|00|aa|bb|cc|dd|ee|ff|
 *      -  -  0  1  2  3  4  5
 */
{
    return uint64_t(addr[0]) << 40 |
        uint64_t(addr[1]) << 32 |
        uint64_t(addr[2]) << 24 |
        uint64_t(addr[3]) << 16 |
        uint64_t(addr[4]) << 8 |
        uint64_t(addr[5]);
}

void mac_addr::parse_mac_string(std::string str_mac)
/*
 * string:   aa:bb:cc:dd:ee:ff
 *
 * big endian (network byte order):
 *     |aa|bb|cc|dd|ee|ff|
 *      0  1  2  3  4  5
 *
 * note: since the string is read forward also the
 * byte array is filled from the beginning of the array.
 */
{
    int byte_index = 0;
    for (int i = 0; (i < mac_length * 3) && (byte_index < mac_length); i++) {
        if (str_mac[i] == ':' || str_mac[i] == '-')
            continue;

        if (!isxdigit(str_mac[i]))
            throw utl_exception("mac_addr ctor: invalid hex digit in string");

        addr[byte_index] = convert_hex_to_int(str_mac[i++]);
        addr[byte_index] <<= 4;
        addr[byte_index] += convert_hex_to_int(str_mac[i]);
        byte_index++;
    }
}

uint8_t mac_addr::convert_hex_to_int(char hex_char)
{
    if (hex_char >= '0' && hex_char <= '9')
        return (uint8_t)(hex_char - '0');

    hex_char = (char)std::toupper(hex_char);
    if (hex_char >= 'A' && hex_char <= 'F')
        return (uint8_t)((hex_char - 'A') + 10);

    throw utl_exception("convert_hex_to_int: invalid hex char");
}

int mac_addr::is_valid_mac_string(const char* mac)
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