#include "mac_addr.h"

mac_addr::mac_addr() : is_hbo(false)
{
    *((uint32_t*)&addr[0]) = 0;
    *((uint16_t*)&addr[4]) = 0;
}

mac_addr::mac_addr(
    uint8_t* addr_in, 
    bool is_host_byte_order) :
        is_hbo(is_host_byte_order)
{
    for (int i = 0; i < mac_length; i++)
        addr[i] = addr_in[i];
}

mac_addr::mac_addr(
    std::string str_mac, 
    bool is_host_byte_order = true) :
        is_hbo(is_host_byte_order)
{
    if (!mac_is_valid(str_mac.c_str()))
        throw utl_exception("mac_addr ctor: invalid mac string");

    int byte_index = 0;
    for (int i = 0; i < mac_length * 3 && byte_index < mac_length; i++) {

        if (str_mac[i] == ':' || str_mac[i] == '-')
            continue;

        if (!isxdigit(str_mac[i]))
            throw utl_exception("mac_addr ctor: invalid hex digit in string");

        addr[byte_index] = hex_to_uint8(str_mac[i++]);
        addr[byte_index] <<= 4;
        addr[byte_index] += hex_to_uint8(str_mac[i]);
        byte_index++;
    }
}

mac_addr::mac_addr(const mac_addr& other) : 
    is_hbo(other.is_hbo)
{
    for (int i = 0; i < mac_length; i++)
        addr[i] = other.addr[i];
}

uint8_t& mac_addr::operator[](int index)
{
    if (index < 0 || index >= mac_length)
        throw utl_exception("mac_addr: index out of bounds");

    return addr[index];
}

bool mac_addr::is_valid() {
    // TODO: this may be implemented better
    return (uint64() != 0);
}

mac_addr mac_addr::ntoh()
{
    mac_addr rval(addr, is_hbo);

    if (is_hbo)
        return rval;

    rval.change_byte_order();
    return rval;
}

mac_addr mac_addr::hton()
{
    mac_addr rval(addr, is_hbo);

    if (!is_hbo)
        return rval;

    rval.change_byte_order();

    return rval;
}
std::string mac_addr::str()
{
    if (is_hbo == false)
        return ntoh().str();

    static const char* digits = "0123456789ABCDEF";
    char rval[18]; // xx:xx:xx:xx:xx:xx'\0'

    int pos = 17;
    for (int byte_index = 5; byte_index >= 0; byte_index--) {
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
{
    uint64_t rval = 0;

    rval += (uint16_t)addr;
    rval <<= 32;
    rval += (uint32_t)addr[2];

    return rval;
}

void mac_addr::change_byte_order()
{
    for (int i = 0; i < mac_length / 2; i++)
    {
        uint8_t tpm = addr[i];
        addr[i] = addr[mac_length - 1 - i];
        addr[mac_length - 1 - i] = tpm;

    }

    is_hbo = !is_hbo;
}

/* Checks if a mac in string format is valid */
int mac_addr::mac_is_valid(const char* mac)
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

uint8_t mac_addr::hex_to_uint8(char hex_char)
{
    if (hex_char >= '0' && hex_char <= '9')
        return (uint8_t)(hex_char - '0');

    hex_char = toupper(hex_char);
    if (hex_char >= 'A' && hex_char <= 'E')
        return (uint8_t)(hex_char - 'A' + 10);

    throw utl_exception("hex_to_uint8: invalid hex char");
}