#include "cameramv.h"
#include <iostream>

using namespace std;

CameraMV::CameraMV(int width, int height): Camera(width,height)
{
}

CameraMV::~CameraMV()
{
}

int CameraMV::init(bool isTrigger)
{
    int CameraCount=devMgr.deviceCount();
    if(CameraCount<2){
        //cout<<"Number of cameras is less than 2!"<<endl;
        errorString="Number of cameras is less than 2!";
        return -1;
    }
    string *CamSerial=new string[CameraCount];
    for(int i=0;i<CameraCount;i++){
        CamSerial[i]=devMgr[i]->serial.read();
        if(CamSerial[i]=="GX000594") //here may be need modification
            pDev[0]=devMgr[i];
        else if(CamSerial[i]=="GX000596")
            pDev[1]=devMgr[i];
    }
    delete [] CamSerial;
    for(int i=0;i<2;i++){
        if(!pDev[i]){ //NULL
            //cout <<"Unable to continue!"<<endl;
            errorString="Unable to continue!";
            return -1;
        }
    }
    for(int i=0;i<2;i++){
        try{
            pDev[i]->open();
        }
        catch( const ImpactAcquireException& e ){
            // this e.g. might happen if the same device is already opened in another process...
            //cout << "An error occurred while opening the device(error code: " << e.getErrorCode() << ")." << endl;
            stringstream ss;ss<<e.getErrorCode();
            errorString="An error occurred while opening the device(error code: "+ss.str()+").";
            return -1;
        }
    }
    if(isTrigger){
        for(int i=0;i<2;i++){
            CameraSettingsBlueFOX bfs( pDev[i] );
            bfs.triggerSource.write(ctsDigIn0); // Uses digital input 0 as the source for the trigger signal
            bfs.triggerMode.write(ctmOnRisingEdge); // Start the exposure of a frame when the trigger input level changes from low to high
            bfs.expose_us.write(5556); // the exposure time for an image in us
        }
    }

    return 0;
}

int CameraMV::connect(int devNum, int *setting, int sizeOfSetting)
{
    return 0;
}

int CameraMV::close()
{
    return 0;
}

int CameraMV::grabSingleFrame(unsigned char *imgBuf, int camIndex)
{
    FunctionInterface fi(pDev[camIndex]);
    SystemSettings ss(pDev[camIndex]);
    const int iMaxRequests=3;
    ss.requestCount.write(iMaxRequests);
    int requestNr;
    bool DoWork=false;
    const int iMaxWaitTime_ms=200; //max waiting time in milliseconds
    int count=0,cntMx=10;
    while((!DoWork) && (count<cntMx)){
        fi.imageRequestSingle(); // send a request to the default request queue of the device and wait for the result.

        requestNr = fi.imageRequestWaitFor( iMaxWaitTime_ms ); // wait for results from the default capture queue

        if( !fi.isRequestNrValid( requestNr ) ) // check if the image has been captured without any problems
             count++;
        else						// If the error code is -2119(DEV_WAIT_FOR_REQUEST_FAILED), the documentation will provide
             DoWork = true;				// additional information under TDMR_ERROR in the interface reference
    }
    if(!DoWork){
        cout << "imageRequestWaitFor failed (" << requestNr << ", " << ImpactAcquireException::getErrorCodeAsString( requestNr ) << ")"
             << ", timeout value too small?" << endl;
        return -1;
    }
    const Request* pRequest;
    DoWork=false;
    count=0;
    while((!DoWork) && (count<cntMx)){
         pRequest = fi.getRequest( requestNr );
         if( !fi.isRequestOK( pRequest ) )
             count++;
         else
             DoWork = true;
    }
    if(!DoWork){
        cout << "Error: " << pRequest->requestResult.readS() << endl;
        return -1;
    }
    //cout << "Image captured( " << pRequest->imagePixelFormat.readS() << " " << pRequest->imageWidth.read() << "x" << pRequest->imageHeight.read() << " )" << endl;
    if(width!=pRequest->imageWidth.read() || height!=pRequest->imageHeight.read()){
        cout<<"Inconsistent image width or height!"<<endl;
        return -1;
    }
    memcpy( imgBuf, (unsigned char *)(pRequest->imageData.read()), sizeof(unsigned char) * (width*height) );
    fi.imageRequestUnlock(requestNr); // unlock the buffer to let the driver know that you no longer need this buffer

    return 0;
}

int CameraMV::grabMultiFrames(unsigned char *imgBuf, int imgNum, int camIndex)
{
    try{
        if(!pDev[camIndex]->isOpen())
            pDev[camIndex]->open();
    }
    catch( const ImpactAcquireException& e ){
        // this e.g. might happen if the same device is already opened in another process...
        cout << "An error occurred while opening the device(error code: " << e.getErrorCode() << ")." << endl;
        return -1;
    }

    const int iMaxWaitTime_ms = 800;
    FunctionInterface fi( pDev[camIndex] );
    SystemSettings    ss(pDev[camIndex]);
    ss.requestCount.write( imgNum ); // set the number of requests allocated by the driver
    for(int i=0; i<imgNum; i++ ){
        fi.imageRequestSingle();
    }
    vector<int> requestNr(imgNum,INVALID_ID);
    for(int i=0; i<imgNum; i++ ){
        requestNr[i] = fi.imageRequestWaitFor( iMaxWaitTime_ms );
    }
    const Request* pRequest = 0;
    for(int i=0; i<imgNum; i++ ){
        if( fi.isRequestNrValid( requestNr[i] ) ){
            pRequest = fi.getRequest( requestNr[i] );
            if( fi.isRequestOK( pRequest ) ){
                memcpy(&imgBuf[i*width*height], (unsigned char*)(pRequest->imageData.read()), width*height);
                fi.imageRequestUnlock(requestNr[i]);
            }
            else
                return -1;
        }
        else
           return -1;
    }

    return 0;
}
