#ifndef RECONSTRUCTOR_H
#define RECONSTRUCTOR_H

#include "imageIO.h"
#include "mat3.h"

#define M_2PI (M_PI*2)

class Reconstructor
{
public:
    Reconstructor(int w,int h)
    {
        width=w;
        height=h;
        size=width*height;
    }

    virtual bool init()=0;
    virtual bool reconstruct(IMAGE3D *image,double *phase,unsigned char *texture,unsigned char *mask)=0;
protected:
    virtual bool paraInit(const std::string &fileName)=0;
    virtual bool readxncam(const std::string &fileName,const std::string &fileNameR)=0;
    virtual Point computeCoor(Point uc,Point up,bool isL2R1)=0;

    int width,height;
    int size;

    Mat3 camLIntrinsic,camRIntrinsic,AA;
    Point camLTrans,camRTrans,TT;
    Mat3 camLRot,camRRot;
};

#endif // RECONSTRUCTOR_H
