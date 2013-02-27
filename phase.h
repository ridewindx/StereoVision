#ifndef PHASE_H
#define PHASE_H

//#include <cmath>
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif
#define M_2PI (M_PI*2)

#define UNWRAPDONE 11

class Phase
{
public:
    Phase(int w,int h,bool if_absolute,double maskThresh,unsigned char *frmPtr[3],double *phase,unsigned char *texture,unsigned char *mask);
    ~Phase();

    void computePhase();
    void convertToAbsolute(unsigned char *centerline,bool isHorizontal);
    void computePhaseWrap();

    void phaseFilter();

protected:
    bool phaseWrapOnly();
    bool phaseWrap();
    bool phaseUnwrap();


    void findHCenterLine(const unsigned char *centerLine, int *center); //寻找水平中心线
    void findVCenterLine(const unsigned char *centerLine, int *center); //寻找垂直中心线
    void convert2Absolute(int *centPtr, const int & lineLength , unsigned char *centerline);

    void compute1DGuass(double *filter,int filtersize);


    int width,height,size;

    double *phx;
    double *phy;
    double maskThresh;
    bool if_absolute;

    bool selectStartPt;
    int idxStart;

    double *guassFilter;
    int guassFilterSize;

    unsigned char *fPtr[3];
    double *phase;
    unsigned char *texture,*mask;
};

#endif // PHASE_H
