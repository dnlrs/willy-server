#ifndef DEVICE_H
#define DEVICE_H


class device
{
private:
    int id;
    float x;
    float y;
public:
    device();
    int getId() const;
    void setId(int value);
    float getX() const;
    void setX(float value);
    float getY() const;
    void setY(float value);
};

#endif // DEVICE_H
