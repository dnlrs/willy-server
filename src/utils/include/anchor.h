#ifndef ANCHOR_H_INCLUDED
#define ANCHOR_H_INCLUDED

#include "utils.h"
#include "mac_addr.h"
#include "ip_addr.h"
#include "point2d.h"
#include <cstdint>
#include <string>
#include <utility>

class anchor
{
public:

    anchor() :
        mac(), ip(), position() {}

    anchor(mac_addr mac, ip_addr ip, double x = 0.0, double y = 0.0) :
        mac(mac), ip(ip), position(x, y) {}

    anchor(mac_addr mac, ip_addr ip, point2d pos) :
        mac(mac), ip(ip), position(pos) {}
    
    anchor(mac_addr mac, ip_addr ip, std::pair<double, double> pos) :
        mac(mac), ip(ip), position(pos) {}


    anchor(const anchor& other) // copy ctor
    {
        mac = other.mac;
        ip  = other.ip;
        position = other.position;
    }

    anchor(anchor&& other) // move ctor
    {
        mac = other.mac;
        ip  = other.ip;
        position = other.position;

        other.mac.clear();
        other.ip.clear();
        other.position.clear();
    }

    anchor& operator=(anchor&& other) // move assign
    {
        mac = other.mac;
        ip  = other.ip;
        position = other.position;

        other.mac.clear();
        other.ip.clear();
        other.position.clear();

        return *this;
    }

    anchor& operator=(const anchor& other) // copy assign
    {
        mac = other.mac;
        ip  = other.ip;
        position = other.position;

        return *this;
    }

    mac_addr get_mac() const { return mac; }
    point2d  get_position() const { return position; }
    double   get_position_x() const { return position.x; }
    double   get_position_y() const { return position.y; };
    
    void set_position(std::pair<double, double> position) 
    { 
        this->position = point2d(position); 
    }

    bool operator<(const anchor& other) const { return mac < other.mac; }

    std::string str() const 
    {
        return std::string(
            "mac: " + mac.str() + 
            " ip: " + ip.str() +
            " pos: " + position.str());
    }

private:
    mac_addr mac;
    ip_addr ip;

    point2d position;

};

#endif // ANCHOR_H_INCLUDED