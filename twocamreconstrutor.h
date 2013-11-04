#ifndef TWOCAMRECONSTRUTOR_H
#define TWOCAMRECONSTRUTOR_H

#include "reconstructor.h"

#include "globals.h"

class TwocamReconstrutor:public Reconstructor
{
public:
    TwocamReconstrutor(int w,int h);
    ~TwocamReconstrutor();

    bool init();

    bool reconstruct(IMAGE3D *image,double *phase,unsigned char *texture,unsigned char *mask);
//private:
    bool paraInit(const std::string &fileName, bool use_old = false);
    bool readxncam(const std::string &fileNameL,const std::string &fileNameR);
    Point computeCoor(Point uc,Point up,bool isL2R1);
    Point computeCoorH(Point uc,Point up,bool isL2R1);

    void findCorrespondence(double hp,double vp,double *phaseVert,double *phaseHori,
                            unsigned char *maskVert,unsigned char *maskHori,
                            int &w,int &h,Point Pi,Point Pj,int io,int jo,
                            double *phaseVertN,unsigned char *maskVertN);

    //double *xncamL,*xncamR;
    float *xncamL,*xncamR;
};

#endif // TWOCAMRECONSTRUTOR_H
