#ifndef SIZED_BUFFER_HPP_INCLUDED
#define SIZED_BUFFER_HPP_INCLUDED

#include "mac_addr.h"
#include <cstdint>
#include <vector>

class sized_buffer
{
public:
    sized_buffer() : msg_size(0) {}

    std::vector<uint8_t> msg;
    uint32_t msg_size;
    mac_addr anchor_mac;
};

#endif // !SIZED_BUFFER_HPP_INCLUDED
