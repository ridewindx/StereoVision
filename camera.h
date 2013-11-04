#ifndef CAMERA_H
#define CAMERA_H

#include <string>

using namespace std;

class Camera
{
public:
    Camera(int width, int height);
    virtual ~Camera();

    virtual int connect(int devNum = 0, int * setting = 0, int sizeOfSetting = 0) = 0;
    virtual int close() = 0;
    virtual int init(bool isTrigger=true) = 0;

    virtual int grabSingleFrame(unsigned char *imgBuf,int camIndex=0)=0;
    virtual int grabMultiFrames(unsigned char *imgBuf, int imgNum=3, int camIndex=0)=0;

    int getWidth();
    int getHeight();

    string getErrorString();

protected:
    int width;
    int height;
    int size;

    string errorString;
};

#endif // CAMERA_H
