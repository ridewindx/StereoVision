#include "combinedmodelmaker.h"

#include "twocamreconstrutor.h"

#include "projection.h"

#include <fstream>

using namespace std;

NineImagesMaker::NineImagesMaker(int w, int h):CombinedModelMaker(w,h)
{
    frmLPhases=new unsigned char[size*9]; //Left nine images
    frmRPhases=new unsigned char[size*9]; //Right nine images
    phase=new double[size*4]; //左边：水平、垂直；右边：水平、垂直
    texture=new unsigned char[size*2]; //左边；右边
    mask=new unsigned char[size*4]; //左边：水平、垂直；右边：水平、垂直
}


bool NineImagesMaker::initialMaker(QDir dir)
{
    QString fileNamesL[9]={"CamLhf0.raw","CamLhf1.raw","CamLhf2.raw","CamLvf0.raw","CamLvf1.raw","CamLvf2.raw","CamLhc.raw","CamLvc.raw","CamLbk.raw"};
    QString fileNamesR[9]={"CamRhf0.raw","CamRhf1.raw","CamRhf2.raw","CamRvf0.raw","CamRvf1.raw","CamRvf2.raw","CamRhc.raw","CamRvc.raw","CamRbk.raw"};
    //string fileNamesL[9]={"0.1600x1200.Mono8.raw","1.1600x1200.Mono8.raw","2.1600x1200.Mono8.raw","3.1600x1200.Mono8.raw","4.1600x1200.Mono8.raw","5.1600x1200.Mono8.raw","6.1600x1200.Mono8.raw","7.1600x1200.Mono8.raw","8.1600x1200.Mono8.raw",};
    //string fileNamesR[9]={"0.1600x1200.Mono8.raw","1.1600x1200.Mono8.raw","2.1600x1200.Mono8.raw","3.1600x1200.Mono8.raw","4.1600x1200.Mono8.raw","5.1600x1200.Mono8.raw","6.1600x1200.Mono8.raw","7.1600x1200.Mono8.raw","8.1600x1200.Mono8.raw",};
    //string fileNamesL[9]={"00.raw","10.raw","20.raw","30.raw","40.raw","50.raw","60.raw","70.raw","80.raw"};
    //string fileNamesR[9]={"01.raw","11.raw","21.raw","31.raw","41.raw","51.raw","61.raw","71.raw","81.raw"};


    QString filePath;
    //dir.cd(QString("LP"));
    for(int i=0;i<9;i++){
        filePath=dir.absoluteFilePath(fileNamesL[i]);
        if(!readBytesFromFile(filePath.toStdString(),&frmLPhases[size*i],size)){
            cerr<<"Reading images error!"<<endl;
            return false;
        }
    }
    //dir.cdUp();
    //dir.cd(QString("RP"));
    for(int i=0;i<9;i++){
        filePath=dir.absoluteFilePath(fileNamesR[i]);
        if(!readBytesFromFile(filePath.toStdString(),&frmRPhases[size*i],size)){
            cerr<<"Reading images error!"<<endl;
            return false;
        }
    }
    //dir.cdUp();

    //中心线图像预处理，去掉背景噪声影响
    int diff;
    for(int i=0;i<size;i++)
    {
        int hc_i= 6*size + i;
        int vc_i= 7*size + i;

        diff=frmLPhases[hc_i]-frmLPhases[8*size+i];
        frmLPhases[hc_i]=diff>0?diff:0;

        diff=frmLPhases[vc_i]-frmLPhases[8*size+i];
        frmLPhases[vc_i]=diff>0?diff:0;

        diff=frmRPhases[hc_i]-frmRPhases[8*size+i];
        frmRPhases[hc_i]=diff>0?diff:0;

        diff=frmRPhases[vc_i]-frmRPhases[8*size+i];
        frmRPhases[vc_i]=diff>0?diff:0;
    }

    //纹理赋值为平均值，即应为纯色光照射下的纹理
    int *tl=new int[size](); //0 value initialize
    int *tr=new int[size]();
    for(int i=0;i<6;i++){
        for(int j=0;j<size;j++){
            tl[j] += frmLPhases[i*size+j];
            tr[j] += frmRPhases[i*size+j];
        }
    }
    for(int j=0;j<size;j++){
        texture[j]=tl[j]/6;
        texture[size+j]=tr[j]/6;
    }
    delete [] tl;
    delete [] tr;

    return true;
}

NineImagesMaker::~NineImagesMaker()
{
    delete [] frmLPhases;
    delete [] frmRPhases;
    delete [] phase;
    delete [] texture;
    delete [] mask;
}

bool NineImagesMaker::make3Dmodel(unsigned char *frmLPhases,unsigned char *frmRPhases,
                                  double *phase,unsigned char *texture,unsigned char *mask,const std::string &image3DName)
{
    if(!calPhase(frmLPhases,frmRPhases,phase,texture,mask))
    {
        cerr<<"Failed in NineImagesMaker::calPhase()!"<<endl;
        return false;
    }
#ifndef NDEBUG
    for(int phaseNo=0;phaseNo<4;phaseNo++){
        double *phase=this->phase+size*phaseNo;
        unsigned char *mask=this->mask+size*phaseNo;
        QString fileName;
        switch(phaseNo){
        case 0:
            fileName="phasemapLeftHori.txt";
            break;
        case 1:
            fileName="phasemapLeftVert.txt";
            break;
        case 2:
            fileName="phasemapRightHori.txt";
            break;
        case 3:
            fileName="phasemapRightVert.txt";
            break;
        }
        QDir dir(workDirectory); //进入工作目录
        std::ofstream outFile(dir.absoluteFilePath(fileName).toStdString().c_str());
        for(int i=0;i<height;i++){
            for(int j=0;j<width;j++){
                if(mask[i*width+j] == UNWRAPDONE)
                    outFile<<phase[i*width+j]<<" ";
                else
                    outFile<<0<<" ";
            }
            outFile<<std::endl;
        }
        outFile.close();
    }
#endif

    IMAGE3D *image=new IMAGE3D[size];
    if(!reconstructTwoCam(phase,&texture[size],mask,image)){ //此处重建，使用的texture是右摄像机的
        cerr<<"Failed in NineImagesMaker::reconstructTwoCam()!"<<endl;
        return false;
    }
    if(!writeSingleImage3D(image3DName,image,width,height)){ //写入文件
        return false;
    }
    delete [] image;
    return true;
}

bool NineImagesMaker::make3Dmodel(const std::string &image3DName)
{
    return(make3Dmodel(frmLPhases,frmRPhases,phase,texture,mask,image3DName));
}

bool NineImagesMaker::readImage(unsigned char * fptrL, unsigned char * fptrR, const int size,
        const std::string& imageName, const int frameNo)
{
    return true;
}

#ifndef NDEBUG
    bool isLeftImage=true; //指示左右图像
#endif

bool NineImagesMaker::calPhase(unsigned char *frmLPhases,unsigned char *frmRPhases,
                               double *phase,unsigned char *texture,unsigned char *mask)
{
    //calculate Left phase
    //horizontal: 水平中心线
#ifndef NDEBUG
    isLeftImage=true;
#endif
    if(!phaseCalculator(&frmLPhases[0],&frmLPhases[6*size],&phase[0],&texture[0],&mask[0],true)){
        cout<<"Calculate left horizontal phase failed!"<<endl;
        return false;
    }
    //vertical: 垂直中心线
#ifndef NDEBUG
    isLeftImage=true;
#endif
    if(!phaseCalculator(&frmLPhases[3*size],&frmLPhases[7*size],&phase[1*size],&texture[0],&mask[1* size],false)){
        cout<<"Calculate left vertical phase failed!"<<endl;
        return false;
    }

    //calculate Right phase
    //horizontal:
#ifndef NDEBUG
    isLeftImage=false;
#endif
    if(!phaseCalculator(&frmRPhases[0],&frmRPhases[6*size],&phase[2*size],&texture[1*size],&mask[2*size],true)){
        cout<<"Calculate right horizontal phase failed!"<<endl;
        return false;
    }
    //vertical:
#ifndef NDEBUG
    isLeftImage=false;
#endif
    if(!phaseCalculator(&frmRPhases[3*size],&frmRPhases[7*size],&phase[3*size],&texture[1*size],&mask[3*size],false)){
        cout<<"Calculate right vertical phase failed!"<<endl;
        return false;
    }
    return true;
}

bool NineImagesMaker::phaseCalculator(unsigned char *frmPhases,unsigned char *centerLine,
                                      double *phase,unsigned char *texture,unsigned char *mask,const bool isHorizontal)
{   
    unsigned char *frmPtr[3];
    for(int i=0;i<3;i++) //three images
        frmPtr[i]=&frmPhases[i*size];

    Phase *phPtr=new Phase(width,height,false,0.07,frmPtr,phase,texture,mask);
    //calculate phase (not absolute)
    phPtr->computePhase();
    //convert to absolute phase
    phPtr->convertToAbsolute(centerLine,isHorizontal);
    for(int i=0;i<size;i++){
        int avg=(frmPtr[1][i]+frmPtr[2][i]+frmPtr[0][i])/3;
        if(avg<5){ //无效
            mask[i]=0;
        }
    }
    delete phPtr;
    return true;
}

bool NineImagesMaker::reconstructTwoCam(double *phase, unsigned char *texture, unsigned char *mask, IMAGE3D *image)
{
    TwocamReconstrutor *rePtr=new TwocamReconstrutor(width,height);
    if(!rePtr->init()){
        return false;
    }
    if(!rePtr->reconstruct(image,phase,texture,mask)){
        return false;
    }
    return true;
    delete rePtr;
}


NineWithGrayCodeMaker::NineWithGrayCodeMaker(int w, int h, int nBits, int offsetH, int offsetV)
    :NineImagesMaker(w,h)
{
    this->nBits=nBits;
    this->offsetH=offsetH;
    this->offsetV=offsetV;
    framesL=new unsigned char[(1+nBits*2)*size];
    framesR=new unsigned char[(1+nBits*2)*size];
    codeL=new int[2*size];
    codeR=new int[2*size];
}

NineWithGrayCodeMaker::~NineWithGrayCodeMaker()
{
    delete framesL;
    delete framesR;
    delete codeL;
    delete codeR;
}

void NineWithGrayCodeMaker::initialMaker(QDir nineimagesDir, QDir graycodeimagesDir)
{
    NineImagesMaker::initialMaker(nineimagesDir);

    QDir dir=graycodeimagesDir;
    dir.cd(QString("GrayCodeImages"));
    string fileNamesGCL[19]={"CamLwt.raw","CamLgch0.raw","CamLgch1.raw","CamLgch2.raw","CamLgch3.raw","CamLgch4.raw","CamLgch5.raw","CamLgch6.raw","CamLgch7.raw","CamLgch8.raw",
                                          "CamLgcv0.raw","CamLgcv1.raw","CamLgcv2.raw","CamLgcv3.raw","CamLgcv4.raw","CamLgcv5.raw","CamLgcv6.raw","CamLgcv7.raw","CamLgcv8.raw"};
    string fileNamesGCR[19]={"CamRwt.raw","CamRgch0.raw","CamRgch1.raw","CamRgch2.raw","CamRgch3.raw","CamRgch4.raw","CamRgch5.raw","CamRgch6.raw","CamRgch7.raw","CamRgch8.raw",
                                          "CamRgcv0.raw","CamRgcv1.raw","CamRgcv2.raw","CamRgcv3.raw","CamRgcv4.raw","CamRgcv5.raw","CamRgcv6.raw","CamRgcv7.raw","CamRgcv8.raw"};

    QString filePath;
    for(int i=0;i<1+nBits*2;i++){
        filePath=dir.absoluteFilePath(QString(fileNamesGCL[i].c_str()));
        readBytesFromFile(filePath.toStdString(),&framesL[i*size],size);

        filePath=dir.absoluteFilePath(QString(fileNamesGCR[i].c_str()));
        readBytesFromFile(filePath.toStdString(),&framesR[i*size],size);
    }
}

void NineWithGrayCodeMaker::grayStripesToBinary(unsigned char *frames, int nBits, unsigned char *frameBK, unsigned char *frameWT, int offset, int *code)
{
    for(int i=0;i<size;i++){
        code[i]=0;
    }
    for(int i=0;i<nBits;i++){
        for(int j=0;j<size;j++){
            if( (frames[i*size+j]-frameBK[j]) < (frameWT[j]-frames[i*size+j]) )
                code[j]=(code[j]<<1) | 0; //Black
            else
                code[j]=(code[j]<<1) | 1; //White
        }
    }
    for(int i=0;i<size;i++){
        code[i]=grayToBinary(code[i])-offset; //Subtract the offset
    }
}

void NineWithGrayCodeMaker::grayStripesToBinary()
{
    grayStripesToBinary(&framesL[size],nBits,&frmLPhases[8],&framesL[0],offsetH,&codeL[0]);
    grayStripesToBinary(&framesL[(1+nBits)*size],nBits,&frmLPhases[8],&framesL[0],offsetV,&codeL[1]);
    grayStripesToBinary(&framesR[size],nBits,&frmRPhases[8],&framesR[0],offsetH,&codeR[0]);
    grayStripesToBinary(&framesR[(1+nBits)*size],nBits,&frmRPhases[8],&framesR[0],offsetV,&codeR[1]);
}

bool NineWithGrayCodeMaker::phaseCalculator(unsigned char *frmPhases,int *code,
                                            double *phase,unsigned char *texture,unsigned char *mask,const bool isHorizontal)
{
    unsigned char *frmPtr[3];
    for(int i=0;i<3;i++) //three images
        frmPtr[i]=&frmPhases[i*size];

    Phase *phPtr=new Phase(width,height,false,0.2,frmPtr,phase,texture,mask);
    //calculate phase (not absolute)
    //phPtr->computePhase();
    phPtr->computePhaseWrap();
    //convert to absolute phase
    //phPtr->convertToAbsolute(centerLine,isHorizontal);
    convertToAbsolute(phase,code,mask,isHorizontal);
    for(int i=0;i<size;i++){
        int avg=(frmPtr[1][i]+frmPtr[2][i]+frmPtr[0][i])/3;
        if(avg<5){ //无效
            mask[i]=0;
        }
    }
    phPtr->phaseFilter();
    delete phPtr;
    return true;
}

void NineWithGrayCodeMaker::convertToAbsolute(double *phase, int *code, unsigned char *mask, bool isHorizontal)
{
    for(int i=0;i<size;i++){
        if(mask[i]==11){
            int n= (isHorizontal?projectionHeight:projectionWidth) -code[i]-1; //from bottom to top --> from top to bottom
            int nc=n/30; //number of periods
            int np=n%30;
            double ph=np/30.0*2.0*M_PI-M_PI;
            if(phase[i]-ph>=5.5)
                phase[i]=phase[i]-M_PI;
            phase[i]=nc*2*M_PI+phase[i];
        }
    }
}



