#include "camera.h"

Camera::Camera(int width, int height)
{
    this->width=width;
    this->height=height;
}

Camera::~Camera()
{
}

int Camera::getWidth()
{
        return width;
}

int Camera::getHeight()
{
    return height;
}

string Camera::getErrorString()
{
    return errorString;
}
