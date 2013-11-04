#ifndef CAMERAPOINTGREY_H
#define CAMERAPOINTGREY_H

#ifdef HAS_POINT_GREY

#include <FlyCapture2.h>

#include "camera.h"

using namespace FlyCapture2;

class CameraPointGrey: public ::Camera
{
public:
    CameraPointGrey(int width, int height);
    ~CameraPointGrey();

    int init(bool isTrigger = true);
    int connect(int devNum = 0, int * setting = 0, int sizeOfSetting = 0); // deprecated
    int close(); // deprecated


    int grabSingleFrame(unsigned char *imgBuf, int camIndex = 0);
    int grabMultiFrames(unsigned char *imgBuf, int imgNum=3, int camIndex = 0);


    void PrintBuildInfo();
    void PrintCameraInfo( CameraInfo* pCamInfo );
    void PrintFormat7Capabilities( Format7Info fmt7Info );
    void PrintError( Error error );
    bool PollForTriggerReady( FlyCapture2::Camera* pCam );


    Error error;
    BusManager busMgr;
    unsigned int numCameras;
    FlyCapture2::Camera** ppCameras;
    Image image;
    Image convertedImage;
};

#endif

#endif // CAMERAPOINTGREY_H
