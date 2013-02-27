#include "phase.h"
#include "globals.h"
#include "imageIO.h"

#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

typedef double lreal;
typedef long int32;

const lreal _double2fixmagic = 68719476736.0*1.5;     //2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
const int32 _shiftamt        = 16;                    //16.16 fixed point representation,

#if BigEndian_
        #define iexp_				0
        #define iman_				1
#else
        #define iexp_				1
        #define iman_				0
#endif //BigEndian_

inline int32 Real2Int(lreal val)
{
#if DEFAULT_CONVERSION
        return val;
#else
        val		= val + _double2fixmagic;
        return ((int32*)&val)[iman_] >> _shiftamt;
#endif
}


Phase::Phase(int w,int h,bool if_absolute,double maskThresh,unsigned char *frmPtr[3],double *phase,unsigned char *texture,unsigned char *mask)
{
    width=w;height=h;size=width*height;
    this->if_absolute=if_absolute; //没有用到
    this->maskThresh=maskThresh;

    fPtr[0] = frmPtr[0];
    fPtr[1] = frmPtr[1];
    fPtr[2] = frmPtr[2];
    this->phase=phase;
    this->texture=texture;
    this->mask=mask;

    phx=new double[size];
    phy=new double[size];

    guassFilterSize=2;
    guassFilter=new double[guassFilterSize*2+1];
    compute1DGuass(guassFilter,guassFilterSize); //高斯高通平滑滤波器

    selectStartPt=false;
}

Phase::~Phase()
{
    delete [] phx;
    delete [] phy;
    delete [] guassFilter;
}

void Phase::compute1DGuass(double *filter,int filtersize)
{
        double sum=0;
        double sigma2=pow(double(filtersize*2/3),2);
        for(int i=0;i<2*filtersize+1;i++){
            filter[i]=exp(-pow(double(i-filtersize),2)/sigma2);
            sum+=filter[i];
        }
        for(int i=0;i<2*filtersize+1;i++)
            filter[i]/=sum; //归一化
}

bool Phase::phaseWrapOnly()
{
    double sqrt3=sqrt(3.0);

    for(int k=0;k<size;k++){
        int I1 = fPtr[0][k];			// fringe0
        int I2 = fPtr[1][k];			// fringe120
        int I3 = fPtr[2][k];			// fringe240
        int Ip = I1+I2+I3;

        if(Ip/3<10){
            mask[k]=10;
        }

        double S = sqrt3*(I3-I2);
        double C = 2.f*I1-(I2+I3);
        phase[k]=atan2(-S,-C);
        if(k==131+200*width){
            cout<<"131:"<<endl;
            cout<<"I1: "<<I1<<endl;
            cout<<"I2: "<<I2<<endl;
            cout<<"I3: "<<I3<<endl;
            cout<<"Phase: "<<phase[k]<<endl;
        }
        if(k==132+200*width){
            cout<<"132:"<<endl;
            cout<<"I1: "<<I1<<endl;
            cout<<"I2: "<<I2<<endl;
            cout<<"I3: "<<I3<<endl;
            cout<<"Phase: "<<phase[k]<<endl;
        }
        if(k==130+200*width){
            cout<<"130:"<<endl;
            cout<<"I1: "<<I1<<endl;
            cout<<"I2: "<<I2<<endl;
            cout<<"I3: "<<I3<<endl;
            cout<<"Phase: "<<phase[k]<<endl;
        }
        if(k==129+200*width){
            cout<<"129:"<<endl;
            cout<<"I1: "<<I1<<endl;
            cout<<"I2: "<<I2<<endl;
            cout<<"I3: "<<I3<<endl;
            cout<<"Phase: "<<phase[k]<<endl;
        }
    }
    return true;
}

bool Phase::phaseWrap()
{
    double *ph = new double[size];

    double sqrt3=sqrt(3.0);

    //maskThresh=0.04;

    for (int k=0;k<size;k++){
        int I1 = fPtr[0][k]; // fringe0
        int I2 = fPtr[1][k]; // fringe120
        int I3 = fPtr[2][k]; // fringe240
        int Ip = I1+I2+I3; //3I'
        double S = sqrt3*(I1-I3); //sine
        double C = 2.f*I2-(I1+I3); //cosine

        double nGamma = C*C+S*S; //(3I'')^2

        phase[k]=atan2(S,C); //新添加的语句

        int t = Real2Int((Ip+powf(nGamma,0.5))/3); //I'+I''
        texture[k]=t<255?t:255; //intensity包络线

        if(nGamma>maskThresh*Ip*Ip){
            if(k>=width){ //非第一行像素
                int I1y = fPtr[0][k-width]; //fringe0
                int I2y = fPtr[1][k-width]; //fringe120
                int I3y = fPtr[2][k-width]; //fringe240
                double Sy = sqrt3*(I1y-I3y); //sine
                double Cy = 2.f*I2y-(I1y+I3y); //cosine
                double deltaS=S*Cy-C*Sy;
                double deltaC=S*Sy+C*Cy;

                phy[k]=atan2(deltaS,deltaC); //与上方像素的y方向相位差值，y方向导数
            }
            else{
                phy[k]=0; //第一行像素y方向相位差值设为0
            }


            if(k%width>0) //非第一列像素
            {
                int I1x = fPtr[0][k-1]; // fringe0
                int I2x = fPtr[1][k-1]; // fringe120
                int I3x = fPtr[2][k-1]; // fringe240
                double Sx = sqrt3*(I1x-I3x); // sine
                double Cx = 2.f*I2x-(I1x+I3x); // cosine
                double deltaS=S*Cx-C*Sx;
                double deltaC=S*Sx+C*Cx;

                phx[k]=atan2(deltaS,deltaC); //与左方像素的x方向相位差值，x方向导数
            }
            else{
                phx[k]=0; //第一列像素x方向相位差值设为0
            }

            ph[k]=powf((phx[k]*phx[k]+phy[k]*phy[k])*0.5,0.5)/M_2PI*2.0; //phase derivative
        }

        if (nGamma>maskThresh*Ip*Ip && ph[k]<0.1){
            mask[k]=ph[k]*100;
            //mask[k]=11;
        }
        else if (nGamma>maskThresh*Ip*Ip && ph[k]<1.0){ //need to be unwrapped
            mask[k]=9;
        }
        else{ //data modulation is not so good, discard it
            mask[k]=10;
        }
    }

    delete [] ph;

#ifndef NDEBUG
    QString xfileName="phaseDerivativeX.txt";
    QString yfileName="phaseDerivativeY.txt";
    QDir dir(workDirectory);
    std::ofstream xoutFile(dir.absoluteFilePath(xfileName).toStdString().c_str());
    std::ofstream youtFile(dir.absoluteFilePath(yfileName).toStdString().c_str());
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            if(mask[i]!=10)
            {xoutFile<<phx[i*width+j]<<" ";youtFile<<phy[i*width+j]<<" ";}
            else
            {xoutFile<<0<<" ";youtFile<<0<<" ";}
        }
        xoutFile<<std::endl;youtFile<<std::endl;
    }
    xoutFile.close();youtFile.close();
#endif

    return true;
}

bool Phase::phaseUnwrap()
{
    double sqrt3=sqrt(3.0);
    int startRow;
    int	midCol=width/2;
    int	midRow=height/2;

    if(selectStartPt){ //idxStart已经指定
        startRow=idxStart/width; //行号
        midCol=idxStart%width; //列号
    }
    else{ //寻找第一个合适的idxStart
        bool start=false;
        idxStart=0;

        for (int j=0;j<width/2;j++,midCol++){
            startRow=midRow;
            for(int i=0;i<height/2;i++,startRow++){
                int k=startRow*width+midCol;
                if(mask[k]<10){
                    start=true;
                    idxStart=k;
                    goto end0;
                }
            }
        }
        end0:
        if (!start){
            midCol=width/2;
            for(int j=0;j<width/2;j++,midCol--){
                startRow=midRow;
                for(int i=0;i<height/2;i++,startRow++){
                    int k=startRow*width+midCol;
                    if(mask[k]<10){
                        start=true;
                        idxStart=k;
                        goto end1;
                    }
                }
            }
        }
        end1:
        if(!start){
            midCol=width/2;
            for (int j=0;j<width/2;j++,midCol++){
                startRow=midRow;
                for(int i=0;i<height/2;i++,startRow--){
                    int k=startRow*width+midCol;
                    if(mask[k]<10){
                        start=true;
                        idxStart=k;
                        goto end2;
                    }
                }
            }
        }
        end2:
        if(!start){
            midCol=width/2;
            for(int j=0;j<width/2;j++,midCol--){
                startRow=midRow;
                for(int i=0;i<height/2;i++,startRow--){
                    int k=startRow*width+midCol;
                    if(mask[k]<10){
                        start=true;
                        idxStart=k;
                        goto end3;
                    }
                }
            }
        }
        end3: ;
    }


    long int *quality[10]; //10个质量等级
    for(int i=0;i<10;i++)
        quality[i]=new long int[size]; //每个质量等级的group可容纳size个元素

    int I1 = fPtr[0][idxStart];	//fringe0
    int I2 = fPtr[1][idxStart];	//fringe120
    int I3 = fPtr[2][idxStart];	//fringe240
    double S = sqrt3*(I1-I3); //sine
    double C = 2.f*I2-(I1+I3); //cosine
    phase[idxStart]=atan2(S,C);

    int s=(int)mask[idxStart]; //s=9

    int index[10]; //存储每个质量等级的group中的待unwrap的元素个数
    for(int i=0;i<10;i++)
        index[i]=0;

    index[s]=1;
    quality[s][index[s]]=idxStart;
    mask[idxStart]=UNWRAPDONE;
    int w=1;

    while(w<10){ //漫水填充算法Flood Fill Algorithm
        int current=quality[s][index[s]];
        index[s]-=1;
        if(current%width>0&&mask[current-1]<10){ //左边像素
            phase[current-1]=phase[current]-phx[current];
            index[(int)mask[current-1]]+=1;
            quality[(int)mask[current-1]][index[(int)mask[current-1]]]=current-1;
            mask[current-1]= UNWRAPDONE;
        }

        if(current%width<width-1&&mask[current+1]<10){ //右边像素
            phase[current+1]=phase[current]+phx[current+1];
            index[(int)mask[current+1]]+=1;
            quality[(int)mask[current+1]][index[(int)mask[current+1]]]=current+1;
            mask[current+1]= UNWRAPDONE;
        }

        if(current/width>0&&mask[current-width]<10){ //上方像素
            phase[current-width]=phase[current]-phy[current];
            index[(int)mask[current-width]]+=1;
            quality[(int)mask[current-width]][index[(int)mask[current-width]]]=current-width;
            mask[current-width]= UNWRAPDONE;
        }

        if(current/width<height-1&&mask[current+width]<10){ //下方像素
            phase[current+width]=phase[current]+phy[current+width];
            index[(int)mask[current+width]]+=1;
            quality[(int)mask[current+width]][index[(int)mask[current+width]]]=current+width;
            mask[current+width]= UNWRAPDONE;
        }

        for(w=0;w<10;w++){
            if (index[w]>0){
                s=w;
                break;
            }
        }
    }

    for(int i=0;i<10;i++)
        delete [] quality[i];

    return true;
}

void Phase::computePhaseWrap()
{
    phaseWrap();
    phaseWrapOnly();
}

void Phase::computePhase()
{
    phaseWrap();
    phaseUnwrap();

    phaseFilter(); //remove high frequency noises
}

//Remove the high frequency noises 高斯滤波,消除高频噪声
void Phase::phaseFilter()
{
    int i,j,k1,m,k;
    int s=guassFilterSize;
    double *F=guassFilter;

    double pSum=0;
    double fSum=0;
    for(i=s;i<height-s;i++){
        for(j=s;j<width-s;j++){
            k1=i*width+j;
            if (mask[k1]==UNWRAPDONE){
                pSum=0.0;
                fSum=0.0;
                for (m=-s;m<=s;m++){
                    k=i*width+j+m; //左右方向
                    if(mask[k]==UNWRAPDONE){
                        pSum+=phase[k]*F[m+s];
                        fSum+=F[m+s];
                    }
                }
            }
            phase[k1]=pSum/fSum;
        }
    }

    for(i=s;i<height-s;i++){
        for(j=s;j<width-s;j++){
            k1=i*width+j;
            if(mask[k1]==UNWRAPDONE){
                pSum=0.0;
                fSum=0.0;
                for(m=-s;m<=s;m++){
                    k=(i+m)*width+j; //上下方向
                    if(mask[k]==UNWRAPDONE){
                        pSum+=phase[k]*F[m+s];
                        fSum+=F[m+s];
                    }
                }
            }
            phase[k1]=pSum/fSum;
        }
    }
}

#ifndef NDEBUG
    extern bool isLeftImage;
#endif
void Phase::convertToAbsolute(unsigned char *centerline,bool isHorizontal)
{
    int lineLength;
    if(isHorizontal)
        lineLength=width;
    else
        lineLength=height;
    int *center =new int[lineLength];

    if (isHorizontal)
        findHCenterLine(centerline,center);
    else
        findVCenterLine(centerline,center);

#ifndef NDEBUG
    unsigned char *image=new unsigned char[size];
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            image[i*width+j]=0;
    for(int i=0;i<lineLength;i++)
        image[center[i]]=255;
    QString fileName=isHorizontal?(isLeftImage?"centerlineLeftHori.raw":"centerlineRightHori.raw"):(isLeftImage?"centerlineLeftVert.raw":"centerlineRightVert.raw");
    QDir dir(workDirectory);
    //writeBytesToFile(dir.absoluteFilePath(fileName).toStdString(),centerline,size);
    writeBytesToFile(dir.absoluteFilePath(fileName).toStdString(),image,size);
    delete [] image;
#endif

    convert2Absolute(center,lineLength,centerline);
}

void Phase::convert2Absolute(int *center,const int &lineLength,unsigned char *centerline)
{
    double tmpPhase=0;
    int count=0;
    for(int i=0;i<lineLength;i++){
        int id=center[i];
        if(mask[id]==UNWRAPDONE && centerline[id]>20){
            tmpPhase+=phase[id];
            count++;
        }
    }

    if(count!=0)
        tmpPhase/=count; //中心线平均相位

    for(int i=0;i<size;i++)
        if(mask[i]==UNWRAPDONE)
            phase[i]=phase[i]-tmpPhase; //减去中心线平均相位，即粗略地设中心线相位为零
}

void Phase::findHCenterLine(const unsigned char *centerLine,int *center)
{
    double max;

    for(int j=0;j<width;j++){ //从上到下，从左到右
        int idx = 0;
        for(int i=guassFilterSize;i<height-guassFilterSize;i++){
            double tmp=0;
            for(int k=-guassFilterSize;k<=guassFilterSize;k++){
                int id=(i+k)*width+j;
                tmp+=centerLine[id]*guassFilter[k+guassFilterSize];
            }
            if(i==guassFilterSize)
                max=tmp; //第一次扫描后，max赋值为第一次扫描的结果
            else if(max<tmp){
                idx=i*width+j; //更新寻找的像素的索引
                max=tmp; //更新max
            }
        }
        center[j]=idx;
    }

}

void Phase::findVCenterLine(const unsigned char *centerLine, int *center)
{
    double max;

    for(int j=0;j<height;j++){ //从左到右，从上到下
        int idx = 0;
        for(int i=guassFilterSize;i<width-guassFilterSize;i++){
            double tmp = 0;
            for(int k=-guassFilterSize;k<=guassFilterSize;k++){
                int id=j*width+(i+k);
                tmp+=centerLine[id]*guassFilter[k+guassFilterSize];
            }
            if(i==guassFilterSize)
                max=tmp;
            else if(max<tmp){
                idx=j*width+i;
                max=tmp;
            }
        }
        center[j]=idx;
    }
}
