#ifndef ANCHOR_H_INCLUDED
#define ANCHOR_H_INCLUDED

#include <cstdint>
#include <utility>

class anchor
{
public:

    anchor() :
        mac(0),
        addr(0),
        position_x(0),
        position_y(0) {}

    anchor(uint64_t mac, uint64_t addr, double x, double y) :
        mac(mac),
        addr(addr),
        position_x(x),
        position_y(y) {}

    anchor(uint64_t mac, uint64_t addr) :
        mac(mac),
        addr(addr),
        position_x(0),
        position_y(0) {}

    anchor(const anchor& other) // copy ctor
    {
        mac = other.mac;
        addr = other.addr;
        position_x = other.position_x;
        position_y = other.position_y;
    }

    anchor(anchor&& other) // move ctor
    {
        mac = other.mac;
        addr = other.addr;
        position_x = other.position_x;
        position_y = other.position_y;

        other.mac = 0;
        other.addr = 0;
        other.position_x = 0;
        other.position_y = 0;
    }

    anchor& operator=(anchor&& other) // move assign
    {
        mac = other.mac;
        addr = other.addr;
        position_x = other.position_x;
        position_y = other.position_y;

        other.mac = 0;
        other.addr = 0;
        other.position_x = 0;
        other.position_y = 0;

        return *this;
    }

    anchor& operator=(const anchor& other)
    {
        mac = other.mac;
        addr = other.addr;
        position_x = other.position_x;
        position_y = other.position_y;

        return *this;
    }

    uint64_t get_mac() const { return mac; }


    bool operator<(const anchor& other) { return mac < other.mac; }

    // TODO: convert to some point type
    std::pair<double, double> get_position() const
    {
        return std::make_pair(position_x, position_y);
    }

    void set_position(std::pair<double, double> position)
    {
        this->position_x = position.first;
        this->position_y = position.second;
    }

    double   get_position_x() const { return position_x; }
    double   get_position_y() const { return position_y; };

private:
    uint64_t mac;
    uint64_t addr;

    double position_x;
    double position_y;

};

#endif // ANCHOR_H_INCLUDED