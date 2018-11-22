#include "position.h"

QString position::getHash() const
{
    return hash;
}

void position::setHash(const QString &value)
{
    hash = value;
}

float position::getX() const
{
    return x;
}

void position::setX(float value)
{
    x = value;
}

float position::getY() const
{
    return y;
}

void position::setY(float value)
{
    y = value;
}

int position::getRssi() const
{
    return rssi;
}

void position::setRssi(int value)
{
    rssi = value;
}

QString position::getSsid() const
{
    return ssid;
}

void position::setSsid(const QString &value)
{
    ssid = value;
}

QString position::getMac() const
{
    return mac;
}

void position::setMac(const QString &value)
{
    mac = value;
}

int position::getChannel() const
{
    return channel;
}

void position::setChannel(int value)
{
    channel = value;
}

float position::getTimestamp() const
{
    return timestamp;
}

void position::setTimestamp(float value)
{
    timestamp = value;
}

int position::getSequence_number() const
{
    return sequence_number;
}

void position::setSequence_number(int value)
{
    sequence_number = value;
}

position::position()
{

}

