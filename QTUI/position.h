#ifndef POSITION_H
#define POSITION_H

#include <QtCore/QString>

class position
{
private:
    QString hash;
    float x;
    float y;
    int rssi;
    QString ssid;
    QString mac;
    int channel;
    float timestamp;
    int sequence_number;
public:
    position();

    QString getHash() const;
    void setHash(const QString &value);
    float getX() const;
    void setX(float value);
    float getY() const;
    void setY(float value);
    int getRssi() const;
    void setRssi(int value);
    QString getSsid() const;
    void setSsid(const QString &value);
    QString getMac() const;
    void setMac(const QString &value);
    int getChannel() const;
    void setChannel(int value);
    float getTimestamp() const;
    void setTimestamp(float value);
    int getSequence_number() const;
    void setSequence_number(int value);
};

#endif // POSITION_H
