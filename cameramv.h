#ifndef CAMERAMV_H
#define CAMERAMV_H

#include "camera.h"
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>

using namespace mvIMPACT::acquire;

class CameraMV : public Camera
{
public:
    CameraMV(int width, int height);
    ~CameraMV();

    int initial(bool isTrigger=true);
    int connect(int devNum = 0, int * setting = 0, int sizeOfSetting = 0);
    int close();

    int grabSingleFrame(unsigned char *imgBuf, int camIndex=0);
    int grabMultiFrames(unsigned char *imgBuf, int imgNum=3, int camIndex=0);

private:
    DeviceManager devMgr;
    Device* pDev[2];
};

#endif // CAMERAMV_H
