#ifdef HAS_POINT_GREY

#include "camerapointgrey.h"

#include <stdio.h>
#include <assert.h>

CameraPointGrey::CameraPointGrey(int width, int height):
    ::Camera(width, height)
{
    /*if (this->init() != 0) {
        printf("Camera Point Grey initialization failed.\n");
    }*/
}

CameraPointGrey::~CameraPointGrey()
{
    for (unsigned int i = 0; i < numCameras; i++ ) {
        error = ppCameras[i]->StopCapture(); // Stop capturing images
        if (error != PGRERROR_OK)
            PrintError( error );

        error = ppCameras[i]->Disconnect(); // Disconnect the camera
        if (error != PGRERROR_OK)
            PrintError( error );

        delete ppCameras[i];
    }
    delete [] ppCameras;
}

int CameraPointGrey::init(bool isTrigger)
{
    PrintBuildInfo();

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK) {
        PrintError(error);
        return -1;
    }
    printf("Number of cameras detected: %u\n", numCameras);
    if (numCameras /*< 1*/ != 2)
    {
        printf( "Insufficient number of cameras! Please check.\n" );
        return -1;
    }

    ppCameras = new FlyCapture2::Camera*[numCameras];
    // Connect to all detected cameras and attempt to set them to
    // a common video mode and frame rate
    for (unsigned int i = 0; i < numCameras; i++) {
        ppCameras[i] = new FlyCapture2::Camera();

        // PGRGuid guid
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK) {
            PrintError(error);
            return -1;
        }

        //停止捕获和连接，以防之前的错误
        ppCameras[i]->StopCapture();
        ppCameras[i]->Disconnect();

        // Connect to a camera
        error = ppCameras[i]->Connect(&guid);
        if (error != PGRERROR_OK) {
            PrintError(error);
            return -1;
        }

        // Get the camera information
        CameraInfo camInfo;
        error = ppCameras[i]->GetCameraInfo(&camInfo);
        if (error != PGRERROR_OK) {
            PrintError(error);
            return -1;
        }
        PrintCameraInfo(&camInfo);

        {
            // Check for external trigger support
            TriggerModeInfo triggerModeInfo;
            error = ppCameras[i]->GetTriggerModeInfo(&triggerModeInfo);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }
            if (triggerModeInfo.present != true) {
                printf( "Camera does not support external trigger!\n" );
                return -1;
            }
            // Get current trigger settings
            TriggerMode triggerMode;
            error = ppCameras[i]->GetTriggerMode(&triggerMode);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }
            // Set camera to trigger mode 0
            triggerMode.onOff = isTrigger; // on or off
            triggerMode.mode = 0;
            triggerMode.parameter = 0;
            triggerMode.source = 0; // Triggering the camera externally using source 0, i.e., GPIO0.
            error = ppCameras[i]->SetTriggerMode(&triggerMode);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }

            if (isTrigger) {
                // Poll to ensure camera is ready
                bool retVal = PollForTriggerReady(ppCameras[i]);
                if (!retVal) {
                    printf("\nError polling for trigger ready!\n");
                    return -1;
                }
            }
        }

        {
            Mode k_fmt7Mode = MODE_0;
            PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;

            // Query for available Format 7 modes
            Format7Info fmt7Info;
            bool supported;
            fmt7Info.mode = k_fmt7Mode;
            error = ppCameras[i]->GetFormat7Info(&fmt7Info, &supported);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }
            PrintFormat7Capabilities( fmt7Info );

            if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0) {
                printf("Pixel format is not supported\n"); // Pixel format not supported!
                return -1;
            }

            Format7ImageSettings fmt7ImageSettings;
            fmt7ImageSettings.mode = k_fmt7Mode;
            fmt7ImageSettings.offsetX = 0;
            fmt7ImageSettings.offsetY = 0;
            fmt7ImageSettings.width = fmt7Info.maxWidth;
            fmt7ImageSettings.height = fmt7Info.maxHeight;
            fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;

            bool valid;
            Format7PacketInfo fmt7PacketInfo;

            // Validate the settings to make sure that they are valid
            error = ppCameras[i]->ValidateFormat7Settings(
                        &fmt7ImageSettings,
                        &valid,
                        &fmt7PacketInfo);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }
            if (!valid ) {
                printf("Format7 settings are not valid\n"); // Settings are not valid
                return -1;
            }

            // Set the settings to the camera
            error = ppCameras[i]->SetFormat7Configuration(
                        &fmt7ImageSettings,
                        //	fmt7PacketInfo.recommendedBytesPerPacket );
                        (unsigned int)516);
            if (error != PGRERROR_OK) {
                PrintError(error);
                return -1;
            }
        }


        VideoMode videoMode = VIDEOMODE_1024x768Y8;
        FrameRate frameRate = FRAMERATE_3_75;
        bool isSupported;
        ppCameras[i]->GetVideoModeAndFrameRateInfo(videoMode, frameRate, &isSupported);
        if (!isSupported) {
            printf("Video mode 1024x768 8bit, Frame rate 3.75, are not supported!\n");
            return -1;
        }
        ppCameras[i]->SetVideoModeAndFrameRate(videoMode, frameRate);



        // Camera is ready, start capturing images
        error = ppCameras[i]->StartCapture();
        if (error != PGRERROR_OK) {
            PrintError( error );
            return -1;
        }

        // Set all cameras to a specific mode and frame rate so they can be synchronized

        // Retrieve frame rate property
        Property frmRate;
        frmRate.type = FRAME_RATE;
        error = ppCameras[i]->GetProperty(&frmRate);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }
        printf( "Framse rate is %3.2f fps\n", frmRate.absValue );


        /*************** property setting ***************/
        Property prop;
        //prop.type = BRIGHTNESS;
        //prop.type = AUTO_EXPOSURE;
        //prop.type = SHARPNESS;

        prop.type = GAMMA;
        ppCameras[i]->GetProperty(&prop);
        prop.absValue = 1.0f;
        ppCameras[i]->SetProperty(&prop, true);

        //prop.type = PAN;
        //prop.type = TILT;

        /*
        prop.type = SHUTTER;
        ppCameras[i]->GetProperty(&prop);
        prop.absValue = 28.0f;
        ppCameras[i]->SetProperty(&prop, true);
        */

        prop.type = GAIN;
        ppCameras[i]->GetProperty(&prop);
        prop.absValue = 0.0f;
        ppCameras[i]->SetProperty(&prop, true);

    }
    return 0;
}

int CameraPointGrey::connect(int devNum, int *setting, int sizeOfSetting)
{
    return 0;
}

int CameraPointGrey::close()
{
    return 0;
}

int CameraPointGrey::grabSingleFrame(unsigned char *imgBuf, int camIndex)
{
    error = ppCameras[camIndex]->RetrieveBuffer(&image);
    if (error != PGRERROR_OK) {
        PrintError(error);
        return -1;
    }
    //assert(image.GetDataSize() == width * height);
    memcpy(imgBuf, image.GetData(), width * height);

    return 0;
}

int CameraPointGrey::grabMultiFrames(unsigned char *imgBuf, int imgNum, int camIndex)
{
    for (int i = 0; i < imgNum; i++) {
        error = ppCameras[camIndex]->RetrieveBuffer(&image);
        if (error != PGRERROR_OK) {
            PrintError(error);
            return -1;
        }
        assert(image.GetDataSize() == width * height);
        memcpy(imgBuf + i * width * height, image.GetData(), width * height);
    }

    return 0;
}

void CameraPointGrey::PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion(&fc2Version);
    char version[128];
    sprintf(version, "FlyCapture2 library version: %d.%d.%d.%d\n\n",
            fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build);
    printf("%s", version);

    /*
    char timeStamp[512];
    sprintf(timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );
    printf("%s", timeStamp);
    */
}

void CameraPointGrey::PrintCameraInfo(CameraInfo *pCamInfo)
{
    printf("\n*** CAMERA INFORMATION ***\n"
           "Serial number - %u\n"
           "Model - %s\n"
           "Vendor - %s\n"
           "Sensor - %s\n"
           "Resolution - %s\n"
           //"Interface - %u\n"
           //"Bus Speed - %u\n"
           "IIDC Version - %u\n"
           "Firmware Version - %s\n"
           "Firmware Build Time - %s\n\n",
           pCamInfo->serialNumber,
           pCamInfo->modelName,
           pCamInfo->vendorName,
           pCamInfo->sensorInfo,
           pCamInfo->sensorResolution,
           //pCamInfo->interfaceType,
           //pCamInfo->maximumBusSpeed,
           pCamInfo->iidcVer,
           pCamInfo->firmwareVersion,
           pCamInfo->firmwareBuildTime
           );
}

void CameraPointGrey::PrintFormat7Capabilities(Format7Info fmt7Info)
{
    printf("Max image pixels: (%u, %u)\n"
           "Image Unit size: (%u, %u)\n"
           "Offset Unit size: (%u, %u)\n"
           "Pixel format bitfield: 0x%08x\n",
           fmt7Info.maxWidth,
           fmt7Info.maxHeight,
           fmt7Info.imageHStepSize,
           fmt7Info.imageVStepSize,
           fmt7Info.offsetHStepSize,
           fmt7Info.offsetVStepSize,
           fmt7Info.pixelFormatBitField
           );
}

void CameraPointGrey::PrintError(Error error)
{
    error.PrintErrorTrace();
}

bool CameraPointGrey::PollForTriggerReady(FlyCapture2::Camera *pCam)
{
    const unsigned int k_softwareTrigger = 0x62C;
    Error error;
    unsigned int regVal = 0;

    do {
        error = pCam->ReadRegister(k_softwareTrigger, &regVal);
        if (error != PGRERROR_OK) {
            PrintError(error);
            return false;
        }
    } while ((regVal >> 31) != 0 );

    return true;
}

#endif
