#ifndef PHASE_H
#define PHASE_H

//#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_2PI
#define M_2PI (M_PI*2)
#endif

#define UNWRAPDONE 11

class Phase
{
public:
    Phase(int w, int h, bool if_absolute, double maskThresh,
          unsigned char *frmPtr[3], double *phase,
          unsigned char *texture,unsigned char *mask);

    ~Phase();

    void computePhase();
    void convertToAbsolute(unsigned char *image_with_centerline, bool isHorizontal);

    void computePhaseWrap(); // deprecated

    void phaseFilter();

protected:
    bool phaseWrapOnly(); // deprecated
    bool phaseWrap();
    bool phaseUnwrap();


    void findHCenterLine(const unsigned char *image_with_centerline, int *centerline_indices); //寻找水平中心线
    void findVCenterLine(const unsigned char *image_with_centerline, int *centerline_indices); //寻找垂直中心线

    void compute1DGuass(double *filter,int filtersize);


    int width, height, size;

    double *phx; // phase derivatives
    double *phy;
    double maskThresh; // threshhold for data modulation
    bool if_absolute; // deprecated

    bool selectStartPt; // deprecated
    int idxStart;

    double *guassFilter;
    int guassFilterSize;

    unsigned char *fPtr[3];

    double *phase; // phase
    unsigned char *texture;
    unsigned char *mask; // quality, 0 ~ 9, or unwrapdown 11
};

#endif // PHASE_H
