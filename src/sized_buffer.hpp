#ifndef SIZED_BUFFER_HPP_INCLUDED
#define SIZED_BUFFER_HPP_INCLUDED

#include <vector>
#include <cstdint>

class sized_buffer {

public:
    sized_buffer() : msg_size(0), anchor_mac(0) {}
    
    std::vector<uint8_t> msg;
    uint32_t msg_size;
    uint64_t anchor_mac;
};


#endif // !SIZED_BUFFER_HPP_INCLUDED