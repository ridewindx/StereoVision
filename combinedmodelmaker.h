#ifndef COMBINEDMODELMAKER_H
#define COMBINEDMODELMAKER_H

#include "imageIO.h"
#include "phase.h"

#include <QtGui>

class CombinedModelMaker
{
public:
    CombinedModelMaker(int w,int h)
    {
        width=w;height=h;size=width*height;
    }

    virtual bool make3Dmodel(unsigned char *frmLPhases,unsigned char *frmRPhases,
                             double *phase,unsigned char *texture,unsigned char *mask,const std::string &image3DName) =0;

    virtual bool readImage(unsigned char *frmLPhases,unsigned char *frmRPhases,const int size,
                           const std::string &image3DName="", const int frameNo=0) =0;

    virtual bool calPhase(unsigned char *frmLPhases,unsigned char *frmRPhases,
                          double *phase,unsigned char *texture,unsigned char *mask) =0;
    int getWidth(){return width;}
    int getHeight(){return height;}
    int getSize(){return size;}

protected:

    int width,height,size;
};

class NineImagesMaker:public CombinedModelMaker
{
public:
    NineImagesMaker(int w,int h);
    ~NineImagesMaker();

    bool make3Dmodel(unsigned char *frmLPhases,unsigned char *frmRPhases,
                     double *phase,unsigned char *texture,unsigned char *mask,const std::string &image3DName);
    bool make3Dmodel(const std::string &image3DName);

    bool readImage(unsigned char *frmLPhases,unsigned char *frmRPhases,const int size,
                   const std::string &image3DName="", const int frameNo=0); //暂时没实现

    bool calPhase(unsigned char *frmLPhases,unsigned char *frmRPhases,
                  double *phase,unsigned char *texture,unsigned char *mask);


    bool phaseCalculator(unsigned char *frmPhases,unsigned char *centerLine,
                         double *phase,unsigned char *texture,unsigned char *mask,const bool isHorizontal);

    bool initialMaker(QDir nineimagesDir);

    double phaseMax(){
        double max=0;
        for(int i=0;i<size*4;i++){
            if(mask[i] == UNWRAPDONE){
                if(max<phase[i])
                    max=phase[i];
            }
        }
        return max;
    }
    double phaseMin(){
        double min=0;
        for(int i=0;i<size*4;i++){
            if(mask[i] == UNWRAPDONE){
                if(min>phase[i])
                    min=phase[i];
            }
        }
        return min;
    }

private:
    bool reconstructTwoCam(double *phase,unsigned char *texture,unsigned char *mask,IMAGE3D *image);

public:
    unsigned char *frmLPhases,*frmRPhases;
    double *phase;
    unsigned char *texture;
    unsigned char *mask;
};

class NineWithGrayCodeMaker:public NineImagesMaker
{
public:
    NineWithGrayCodeMaker(int w,int h, int nBits, int offsetH, int offsetV);
    ~NineWithGrayCodeMaker();

    void initialMaker(QDir nineimagesDir, QDir graycodeimagesDir);

    void grayStripesToBinary(unsigned char *frames, int nBits, unsigned char *frameBK, unsigned char *frameWT, int offset, int *code);
    void grayStripesToBinary();

    unsigned int grayToBinary(unsigned int num)
    {   //Convert gray code to binary code
        unsigned int numBits = 8*sizeof(num);
        for (unsigned int shift = 1; shift < numBits; shift = 2*shift)
        {
            num = num ^ (num >> shift);
        }
        return num;
    }

    unsigned int binaryToGray(unsigned int num)
    {   //Convert binary code to gray code
        return (num >> 1) ^ num;
    }

    bool phaseCalculator(unsigned char *frmPhases, int *code,
                         double *phase, unsigned char *texture, unsigned char *mask, const bool isHorizontal);
    void convertToAbsolute(double *phase, int *code, unsigned char *mask, bool isHorizontal);

private:
    unsigned char *framesL;
    unsigned char *framesR;
    int nBits;
    int offsetH;
    int offsetV;

public:
    int *codeL;
    int *codeR;
};

#endif // COMBINEDMODELMAKER_H
