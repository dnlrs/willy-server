#include "device.h"

int device::getId() const
{
    return id;
}

void device::setId(int value)
{
    id = value;
}

float device::getX() const
{
    return x;
}

void device::setX(float value)
{
    x = value;
}

float device::getY() const
{
    return y;
}

void device::setY(float value)
{
    y = value;
}

device::device()
{

}
