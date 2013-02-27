#include "twocamreconstrutor.h"
#include <iostream>
#include <fstream>

#include <QtGUI>

#define UNWRAPDONE 11

using namespace std;

TwocamReconstrutor::TwocamReconstrutor(int w,int h):Reconstructor(w,h)
{
    //xncamL=new double[size*2];
    //xncamR=new double[size*2];
    xncamL=new float[size*2];
    xncamR=new float[size*2];
}

TwocamReconstrutor::~TwocamReconstrutor()
{
    delete [] xncamL;
    delete [] xncamR;
}

bool TwocamReconstrutor::init()
{
    QDir dir(workDirectory); //进入工作目录
    dir.mkdir("TwoCamCalibData"); //建立"TwoCamCalibData"目录，或者已经存在
    //QString dirSelected=QFileDialog::getExistingDirectory(NULL,"Select Directory with file \"CalibParameters.txt\"",dir.absolutePath()); //选择目录对话框
    //if(!dirSelected.isEmpty()){ //选择的目录
    //    dir.setPath(dirSelected);
    //}
    //else{ //若点击“取消”按钮，则选择默认目录
        dir.cd(QString("TwoCamCalibData"));
    //}

    if(!( dir.exists("CalibParameters.txt") && dir.exists("xncamL.txt") && dir.exists("xncamR.txt") )){
        QMessageBox::warning(NULL,QString("File finding error!"),QString("\"CalibParameters.txt\", \"xncamL.txt\", \"xncamL.txt\" don\'t exists!"));
        return false;
    }

    QString paraFile=dir.absoluteFilePath(QString("CalibParameters.txt"));
    QString xncamLFile=dir.absoluteFilePath(QString("xncamL.txt"));
    QString xncamRFile=dir.absoluteFilePath(QString("xncamR.txt"));
    if(!paraInit(paraFile.toStdString()) || !readxncam(xncamLFile.toStdString(),xncamRFile.toStdString())){
        QMessageBox::warning(NULL,QString("Opening file error!"),QString("Opening file error!"));
        return false;
    }
    return true;
}

bool TwocamReconstrutor::paraInit(const std::string &fileName)
{
    ifstream inFile(fileName.c_str());
    if(!inFile.is_open()){
        cerr<<"Failed to open "<<fileName<<" for reading"<<endl;
        return false;
    }
    double camLFL[2],camRFL[2]; //Focal Length
    double camLPP[2],camRPP[2]; //Principle Point

    //Left camera instrinsic parameters
    for(int i=0;i<3;i++)
        inFile.ignore(255,'\n');
    inFile>>camLFL[0];inFile>>camLFL[1];
    for(int i=0;i<2;i++)
        inFile.ignore(255,'\n');
    inFile>>camLPP[0];inFile>>camLPP[1];

    //Right camera instrinsic parameters
    for(int i=0;i<3;i++)
        inFile.ignore(255,'\n');
    inFile>>camRFL[0];inFile>>camRFL[1];
    for(int i=0;i<2;i++)
        inFile.ignore(255,'\n');
    inFile>>camRPP[0];inFile>>camRPP[1];

    //Left camera extrinsic parameters
    for(int i=0;i<5;i++)
        inFile.ignore(255,'\n');
    //Translation
    for(int i=0;i<3;i++)
        inFile>>camLTrans[i];
    for(int i=0;i<2;i++)
        inFile.ignore(255,'\n');
    //Rotation
    for(int i=0;i<9;i++)
        inFile>>camLRot.m[i/3][i%3]; //按行读取

    //Right camera extrinsic parameters
    for(int i=0;i<3;i++)
        inFile.ignore(255,'\n');
    //Translation
    for(int i=0;i<3;i++)
        inFile>>camRTrans[i];
    for(int i=0;i<2;i++)
        inFile.ignore(255,'\n');
    //Rotation
    for(int i=0;i<9;i++)
        inFile>>camRRot.m[i/3][i%3]; //按行读取

    inFile.close();

    double mL[3][3]={ {camLFL[0],         0, camLPP[0]},
                      {        0, camLFL[1], camLPP[1]},
                      {        0,         0,         1} };
    camLIntrinsic=Mat3(mL);
    double mR[3][3]={ {camRFL[0],         0, camRPP[0]},
                      {        0, camRFL[1], camRPP[1]},
                      {        0,         0,         1} };
    camRIntrinsic=Mat3(mR);

    AA=camRIntrinsic*camRRot*camLRot.inverse()*camLIntrinsic.inverse();
    TT=camRIntrinsic*camRTrans-camRIntrinsic*camRRot*camLRot.inverse()*camLTrans;

    //AA=camLIntrinsic*camLRot*camRRot.inverse()*camRIntrinsic.inverse();
    //TT=camLIntrinsic*camLTrans-camLIntrinsic*camLRot*camRRot.inverse()*camRTrans;

    return true;
}

bool TwocamReconstrutor::readxncam(const std::string &fileNameL,const std::string &fileNameR)
{
    ifstream inFile;
    inFile.open(fileNameL.c_str(),ios::binary);
    if(!inFile.is_open()){
        cerr<<"Failed to open "<<fileNameL<<" for reading"<<endl;
        return false;
    }
    inFile.read((char *)xncamL,size*2*sizeof(xncamL[0]));
    inFile.close();

    inFile.open(fileNameR.c_str(),ios::binary);
    if(!inFile.is_open()){
        cerr<<"Failed to open "<<fileNameR<<" for reading"<<endl;
        return false;
    }
    inFile.read((char *)xncamR,size*2*sizeof(xncamR[0]));
    inFile.close();

    return true;
}

bool TwocamReconstrutor::reconstruct(IMAGE3D *image,double *phase,unsigned char *texture,unsigned char *mask)
{
    double *phaseHoriL=phase;
    double *phaseVertL=&phase[size];
    double *phaseHoriR=&phase[size*2];
    double *phaseVertR=&phase[size*3];
    unsigned char *maskHoriL=mask;
    unsigned char *maskVertL=&mask[size];
    unsigned char *maskHoriR=&mask[size*2];
    unsigned char *maskVertR=&mask[size*3];

    //int th=0,tw=0;
    for (int j=0;j<height;j++){ //begin for 1
    //for(int i=0;i<width;i++){
        Point Pi(0,width); //搜索区域的左右范围
        Point Pj(0,height); //搜索区域的上下范围

        for(int i=0;i<width;i++){ //begin for 2
        //for (int j=0;j<height;j++){
            int idx=i+j*width;

            //if the point is valid in the right camera, then we try to find it in the left camera
            if(maskHoriR[idx]==UNWRAPDONE && maskVertR[idx]==UNWRAPDONE){ //begin if 1
            //if(maskVertR[idx]==UNWRAPDONE){
                bool iffind = true;

                /*********find epipolar line to narrow the searching area*********/
                Point xy1=AA*(600)*Point(i,j,1)+TT;
                xy1=xy1/xy1[2]; //直线上一点

                Point xy2=AA*(1000)*Point(i,j,1)+TT;
                xy2=xy2/xy2[2]; //直线上另一点

                double k=(xy1[1]-xy2[1])/(xy1[0]-xy2[0]); //直线斜率
                double newj=(i-xy1[0])*k+xy1[1]; //直线上横坐标为i的点
                //double newi=(j-xy1[1])/k+xy1[0];

                //searching area band搜索带宽的一半
                int band=35;
                //int band=15;
                //int band=75;


                if(newj-band>=height || newj-band>=Pj[1]){
                    iffind=false;
                }
                else if(newj+band<=0 || newj+band<=Pj[0]){
                    iffind=false;
                }
                else{
                    if(newj-band>0 && newj-band>Pj[0])
                        Pj[0]=newj-band;
                    if(newj+band<height && newj+band<Pj[1])
                        Pj[1]=newj+band;
                    if(Pj[0]==Pj[1]){
                        if(Pj[0]>0+band && Pj[1]<height-band){
                            Pj[0]-=band;
                            Pj[1]+=band;
                        }
                        else
                            ; //iffind = false;
                    }
                }

                /*
                if(newi-band>=width || newi-band>=Pi[1]){
                    iffind=false;
                }
                else if(newi+band<=0 || newi+band<=Pi[0]){
                    iffind=false;
                }
                else{
                    if(newi-band>0 && newi-band>Pi[0])
                        Pi[0]=newi-band;
                    if(newi+band<width && newi+band<Pi[1])
                        Pi[1]=newi+band;
                    if(Pi[0]==Pi[1]){
                        if(Pi[0]>0+band && Pi[1]<width-band){
                            Pi[0]-=band;
                            Pi[1]+=band;
                        }
                        else
                            ; //iffind = false;
                    }
                }
                */
                /*********epipolar line found*********/

                double hp = phaseHoriR[idx];
                double vp = phaseVertR[idx];

                int tempw=i,temph=j; //寻找匹配像素过程的输出参数，即匹配像素的坐标(在搜索区域内phase值最接近)
                if(iffind){ //find the corresponding pixel in left camera
                    findCorrespondence(vp,hp,phaseVertL,phaseHoriL,maskVertL,maskHoriL,
                                       tempw,temph,Pi,Pj,i,j,phaseVertR,maskVertR);
                                       //tempw,temph,Pi,Pj,i,j,phaseHoriR,maskHoriR);
                }
                else{ //无效点
                    image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                    image[idx].mask=false;
                    continue;
                }

                if(temph<0||temph>height||tempw<0||tempw>width){ //无效点
                    image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                    image[idx].mask=false;
                    continue;
                }

                int id=(temph*width+tempw);

                //double thresh=0.2; //判断相似的阈值
                //double thresh=0.1;
                //double thresh=0.4;
                double thresh=2;

                //if the found pixel has phase value similar to hp, vp, means it's correct pixel
                if (fabs(phaseHoriL[id]-hp)<thresh && fabs(phaseVertL[id]-vp)<thresh){ //begin if 2

                    //begin filter
                    int lu,ld,ru,rd,ltor,utod;
                    if(phaseHoriL[id]-hp>0)
                        ltor=-1;
                    else
                        ltor=0;
                    if(phaseVertL[id]-vp>0)
                        utod=-1;
                    else
                        utod=0;
                    /*
                    ._.______.
                       \.   |.
                        \   |
                    .____\._|.
                    */
                    lu=id+ltor*width+utod; //左上角
                    ld=lu+1; //右上角
                    ru=lu+width; //左下角
                    rd=ru+1; //右下角

                    double newtempw=tempw; //亚像素级别
                    double newtemph=temph; //亚像素级别
                    //double pthresh=1.0;
                    //double pthresh=0.75;
                    //double pthresh=1e9;
                    double pthresh=5;
                    if (lu>width && lu<size-width
                        && fabs(phaseHoriL[lu]-hp)<pthresh
                        && fabs(phaseHoriL[ld]-hp)<pthresh
                        && fabs(phaseHoriL[ru]-hp)<pthresh
                        && fabs(phaseHoriL[rd]-hp)<pthresh
                        && fabs(phaseVertL[lu]-vp)<pthresh
                        && fabs(phaseVertL[ld]-vp)<pthresh
                        && fabs(phaseVertL[ru]-vp)<pthresh
                        && fabs(phaseVertL[rd]-vp)<pthresh
                        )
                    {
                        double l=(phaseVertL[lu]-vp)/(phaseVertL[lu]-phaseVertL[ld]);
                        double r=(phaseVertL[ru]-vp)/(phaseVertL[ru]-phaseVertL[rd]);
                        l=l>0?l:0;l=l<1?l:1;r=r>0?r:0;r=r<1?r:1;
                        double uplineP = (tempw+utod) +l;
                                //+ (phaseVertL[lu]-vp)/(phaseVertL[lu]-phaseVertL[ld]);
                        double downlineP = (tempw+utod) +r;
                                //+ (phaseVertL[ru]-vp)/(phaseVertL[ru]-phaseVertL[rd]);
                        double base = -phaseHoriL[rd]-phaseHoriL[ru]+phaseHoriL[ld]+phaseHoriL[lu];
                        double upratio = ((phaseHoriL[lu]-hp)+(phaseHoriL[ld]-hp))/base;
                        upratio=upratio>0?upratio:0;upratio=upratio<1?upratio:1;
                        //double downratio = -((phaseHoriL[ru]-hp)+(phaseHoriL[rd]-hp))/base; //upratio+downratio=1
                        double downratio=1-upratio;
                        //newtemph=temph+upratio;

                        //if(fabs((phaseVertL[lu]-vp)/(phaseVertL[lu]-phaseVertL[ld]))<1.2
                        //&& fabs((phaseVertL[ru]-vp)/(phaseVertL[ru]-phaseVertL[rd]))<1.2
                        //&& fabs(upratio)<2.0
                        //&& fabs(downratio)<2.0) //2.0 is experimental value
                        {
                            newtempw=uplineP*downratio + downlineP*upratio; // //直线上三点OC=(1-s)*OA+s*OB，线性插值得到亚像素级别的横坐标
                        }
                    }
                    else{ //无效点
                        image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                        image[idx].mask=false;
                        continue;
                    }

                    lu=id+ltor*width+utod; //左上角
                    ru=lu+1; //右上角
                    ld=lu+width; //左下角
                    rd=ru+1; //右下角


                    if (lu>width && lu<size-width
                        && fabs(phaseHoriL[lu]-hp)<pthresh
                        && fabs(phaseHoriL[ld]-hp)<pthresh
                        && fabs(phaseHoriL[ru]-hp)<pthresh
                        && fabs(phaseHoriL[rd]-hp)<pthresh
                        && fabs(phaseVertL[lu]-vp)<pthresh
                        && fabs(phaseVertL[ld]-vp)<pthresh
                        && fabs(phaseVertL[ru]-vp)<pthresh
                        && fabs(phaseVertL[rd]-vp)<pthresh
                        )
                    {   double l=(phaseHoriL[lu]-hp)/(phaseHoriL[lu]-phaseHoriL[ld]);
                        double r=(phaseHoriL[ru]-hp)/(phaseHoriL[ru]-phaseHoriL[rd]);
                        l=l>0?l:0;l=l<1?l:1;r=r>0?r:0;r=r<1?r:1;
                        double uplineP = (temph+ltor) +l;
                                //+ (phaseHoriL[lu]-hp)/(phaseHoriL[lu]-phaseHoriL[ld]);
                        double downlineP = (temph+ltor) +r;
                                //+ (phaseHoriL[ru]-hp)/(phaseHoriL[ru]-phaseHoriL[rd]);
                        double base = -phaseVertL[rd]-phaseVertL[ru]+phaseVertL[ld]+phaseVertL[lu];
                        double upratio = ((phaseVertL[lu]-vp)+(phaseVertL[ld]-vp))/base;
                        upratio=upratio>0?upratio:0;upratio=upratio<1?upratio:1;
                        //double downratio = -((phaseVertL[ru]-vp)+(phaseVertL[rd]-vp))/base; //upratio+downratio=1
                        double downratio=1-upratio;
                        //newtempw=(newtempw+(tempw+upratio))/2;

                        //if(fabs((phaseHoriL[lu]-hp)/(phaseHoriL[lu]-phaseHoriL[ld]))<1.2
                        //&& fabs((phaseHoriL[ru]-hp)/(phaseHoriL[ru]-phaseHoriL[rd]))<1.2
                        //&& fabs(upratio)<2.0
                        //&& fabs(downratio)<2.0) //2.0 is experimental value
                        {
                            newtemph=uplineP*downratio + downlineP*upratio; // //直线上三点OC=(1-s)*OA+s*OB，线性插值得到亚像素级别的横坐标
                            //newtemph=(newtemph+(uplineP*downratio + downlineP*upratio))/2;
                        }
                        //else
                            //newtemph=temph;
                    }
                    else{ //无效点
                        image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                        image[idx].mask=false;
                        continue;
                    }
                    //end filter

                    /**********************************************************************/
                    int tht=newtempw; //truncate to int, tht<=newtempw
                    if(!(tht>=0&&tht<width-1)){ //无效点
                        image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                        image[idx].mask=false;
                        continue;
                    }
                    double thu = xncamL[int(temph + tht*height)*2]; //右摄像机坐标系规范化坐标，Xc/Zc
                    double thd = xncamL[int(temph + (tht+1)*height)*2]; //右边像素点，Xc/Zc
                    newtempw = (tht+1-newtempw)*thu+(newtempw-tht)*thd; //直线上三点OC=(1-s)*OA+s*OB，线性插值得到亚像素点对应的规范化坐标

                    tht=newtemph; //truncate to int, tht<=newtemph
                    if(!(tht>=0&&tht<height-1)){ //无效点
                        image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                        image[idx].mask=false;
                        continue;
                    }
                    thu = xncamL[int(tht + tempw*height)*2+1]; //右摄像机坐标系规范化坐标，Yc/Zc
                    thd = xncamL[int(tht+1 + tempw*height)*2+1]; //下边像素点，Yc/Zc
                    newtemph = (tht+1-newtemph)*thu+(newtemph-tht)*thd; //直线上三点OC=(1-s)*OA+s*OB，线性插值得到亚像素点对应的规范化坐标
                    /**********************************************************************/

                    double uc=xncamR[(i*height+j)*2]; //右摄像机坐标系规范化坐标，Xc/Zc,Yc/Zc
                    double vc=xncamR[(i*height+j)*2+1];

                    //newtempw=xncamL[int(temph + tempw*height)*2];
                    //newtemph=xncamL[int(temph + tempw*height)*2+1];
                    Point uc0(uc,vc,1);
                    Point up0(newtempw,newtemph,1);

                    image[idx].coor=computeCoor(uc0,up0,1); //空间点的三维坐标
                    //image[idx].coor=computeCoor(uc0,up0,0);

                    image[idx].text[0]=image[idx].text[1]=image[idx].text[2]=texture[idx]/255.f; //纹理intensity，0~1
                    //if((image[idx].coor[2]>-1000) && (image[idx].coor[2]<1000))
                        image[idx].mask=true;
                    //else
                        //image[idx].mask=false;

                    //缩小同一行上的下一个待匹配像素的搜索范围
                    int win=5;
                    //int win=20;
                    Pi[0]=(tempw-win)>0?(tempw-win):0;
                    Pi[1]=(tempw+win)<width?(tempw+win):width;
                    Pj[0]=(temph-win)>0?(temph-win):0;
                    Pj[1]=(temph+win)<height?(temph+win):height;
                } //end if 2
                else{ //无效点
                    image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                    image[idx].mask=false;

                    Pi[0]=0;
                    Pi[1]=width;
                    Pj[0]=0;
                    Pj[1]=height;
                }
            } //end if 1
            else{ //无效点
                image[idx].coor[0]=image[idx].coor[1]=image[idx].coor[2]=0;
                image[idx].mask=false;

                Pi[0]=0;
                Pi[1]=width;
                Pj[0]=0;
                Pj[1]=height;
            }
        } //end for 2
    } //end for 1

    return true;
}

/* 求取左右图像中某一对匹配的像素点对应的空间点的三维坐标，世界坐标系由输入参数中的Rl,Tl(或Rr,Tr)决定
   [Xc,Yc,Zc]'=(R,T)*[Xw,Yw,Zw,1]'
   xn=Xc/Zc,yn=Yc/Zc --> xn*Zc-Xc=0,yn*Zc-Yc=0
   uc取用xn,yn, 而up只取用xn, 三个方程联立线性方程组求解三个未知数
*/
Point TwocamReconstrutor::computeCoor(Point uc,Point up,bool isL2R1)
{
    Mat3 A;
    Point b;
    double r[4][3];double t[4];
    if(isL2R1){ //左2右1
        for(int k=0;k<3;k++){
            for(int i=0;i<2;i++){
                //A.m[i][k]=uc[i]*camLRot.m[2][k]-camLRot.m[i][k];
                //b[i]=camLTrans[i]-uc[i]*camLTrans[2];
                r[i][k]=uc[i]*camLRot.m[2][k]-camLRot.m[i][k];
                t[i]=camLTrans[i]-uc[i]*camLTrans[2];
            }
            //A.m[2][k]=up[0]*camRRot.m[2][k]-camRRot.m[0][k];
            //b[2]=camRTrans[0]-up[0]*camRTrans[2];
            //A.m[2][k]=up[1]*camRRot.m[2][k]-camRRot.m[1][k];
            //b[2]=camRTrans[1]-up[1]*camRTrans[2];
            for(int i=0;i<2;i++){
                r[i+2][k]=up[i]*camRRot.m[2][k]-camRRot.m[i][k];
                t[i+2]=camRTrans[i]-up[i]*camRTrans[2];
            }
        }
    }
    else{ //右2左1
        for(int k=0;k<3;k++){
            for(int i=0;i<2;i++){
                //A.m[i][k]=uc[i]*camRRot.m[2][k]-camRRot.m[i][k];
                //b[i]=camRTrans[i]-uc[i]*camRTrans[2];
                r[i][k]=uc[i]*camRRot.m[2][k]-camRRot.m[i][k];
                t[i]=camRTrans[i]-uc[i]*camRTrans[2];
            }
            //A.m[2][k]=up[0]*camLRot.m[2][k]-camLRot.m[0][k];
            //b[2]=camLTrans[0]-up[0]*camLTrans[2];
            //A.m[2][k]=up[1]*camLRot.m[2][k]-camLRot.m[1][k];
            //b[2]=camLTrans[1]-up[1]*camLTrans[2];
            for(int i=0;i<2;i++){
                r[i+2][k]=up[i]*camLRot.m[2][k]-camLRot.m[i][k];
                t[i+2]=camLTrans[i]-up[i]*camLTrans[2];
            }
        }
    }
    /*
    static int i0=2,i1=1,i2=3;
    for(int k=0;k<3;k++){
            A.m[0][k]=r[i0][k];
            A.m[1][k]=r[i1][k];
            A.m[2][k]=r[i2][k];
    }
    b[0]=t[i0];b[1]=t[i1];b[2]=t[i2];

    return A.inverse()*b; //A*X=b
    */

    //最小二乘法, Ax=b --> x=inv(A'A)*A'*b
    double rT[3][4];
    for(int i=0;i<3;i++){
        for(int j=0;j<4;j++){
            rT[i][j]=r[j][i];
        }
    }
    Mat3 rTr;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            rTr.m[i][j]=rT[i][0]*r[0][j]+rT[i][1]*r[1][j]+rT[i][2]*r[2][j]+rT[i][3]*r[3][j];
        }
    }
    Point rTt;
    for(int i=0;i<3;i++){
        rTt[i]=rT[i][0]*t[0]+rT[i][1]*t[1]+rT[i][2]*t[2]+rT[i][3]*t[3];
    }
    return rTr.inverse()*rTt;
}

Point T(Point &T){
    Point camRTrans=T;
    for(int i=0;i<3;i++){
        cout<<camRTrans[i]<<" ";
    }
    cout<<endl;

    double t=camRTrans[0];camRTrans[0]=camRTrans[1];camRTrans[1]=t;

    for(int i=0;i<3;i++){
        cout<<camRTrans[i]<<" ";
    }
    cout<<endl;
    return camRTrans;
}
Mat3 R(Mat3 &R){
    Mat3 camRRot=R;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            cout<<camRRot.m[i][j]<<" ";
        }
        cout<<endl;
    }

    double t=camRRot.m[0][0];camRRot.m[0][0]=camRRot.m[1][0];camRRot.m[1][0]=t;
    t=camRRot.m[0][1];camRRot.m[0][1]=camRRot.m[1][1];camRRot.m[1][1]=t;
    t=camRRot.m[0][2];camRRot.m[0][2]=camRRot.m[1][2];camRRot.m[1][2]=t;

    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            cout<<camRRot.m[i][j]<<" ";
        }
        cout<<endl;
    }
    return camRRot;
}

Point TwocamReconstrutor::computeCoorH(Point uc,Point up,bool isL2R1)
{
    //static Point camRTrans=T(this->camRTrans);
    //static Mat3 camRRot=R(this->camRRot);

    Mat3 A;
    Point b;Point x;
    if(isL2R1){ //左2右1
        for(int k=0;k<3;k++){
            for(int i=0;i<2;i++){
                A.m[i][k]=uc[i]*camLRot.m[2][k]-camLRot.m[i][k];
                b[i]=camLTrans[i]-uc[i]*camLTrans[2];
            }
            A.m[2][k]=up[1]*camRRot.m[2][k]-camRRot.m[1][k];
            b[2]=camRTrans[1]-up[1]*camRTrans[2];
        }
        /*
        for(int i=0;i<3;i++){
            cout<<b[i]<<" ";
        }
        cout<<endl;
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                cout<<A.m[i][j]<<" ";
            }
            cout<<endl;
        }
        */
    }
    else{ //右2左1
        for(int k=0;k<3;k++){
            for(int i=0;i<2;i++){
                A.m[i][k]=uc[i]*camRRot.m[2][k]-camRRot.m[i][k];
                b[i]=camRTrans[i]-uc[i]*camRTrans[2];
            }
            A.m[2][k]=up[1]*camLRot.m[2][k]-camLRot.m[1][k];
            b[2]=camLTrans[1]-up[1]*camLTrans[2];
        }
    }
    x=A.inverse()*b;
    //std::cout<<x.value[0]<<"  "<<x.value[1]<<"  "<<x.value[2]<<std::endl;
    /*
    double d=0;
    for(int k=0;k<3;k++){
        d+=(up[0]*camRRot.m[2][k]-camRRot.m[0][k])*x.value[k];
    }
    d-=camRTrans[0]-up[0]*camRTrans[2];
    std::cout<<d<<endl;
    */
    return x;
    //return A.inverse()*b; //A*X=b
}

/* 寻找指定像素的匹配像素。
   一个phase map：io,jo为待匹配像素点列索引、行索引，
                 vp,hp为这个像素点的垂直和水平pattern的phase
                 phaseVertN,maskVertN为垂直pattern的phase和mask
   另一个phase map：phaseVert,phaseHori,maskVert,maskHori为垂直和水平pattern的phase和mask
                 Pi、Pj包含水平和垂直方向的搜索范围，Pi[0]<=x<Pi[1],Pj[0]<=y<Pj[1]
                 w和h为输出参数，赋值为搜索到的匹配点的列索引和行索引
*/
void TwocamReconstrutor::findCorrespondence(double vp,double hp,double *phaseVert,double *phaseHori,
                                            unsigned char *maskVert,unsigned char *maskHori,
                                            int &w,int &h,Point Pi,Point Pj,int io,int jo,
                                            double *phaseVertN,unsigned char *maskVertN)
{
    int *vertitemp=new int[height];
    int ido=io+jo*width;

    for(int j=Pj[0];j<Pj[1];j++){ //从上到下的搜索范围
        vertitemp[j]=0;
        double min=255;
        int start=Pi[0]; //从左到右的搜索范围，开始，结束
        int end=Pi[1];

        while(end-start>1){ //对半查找法
            int middle=(start+end+1)/2; //中点
            int id=middle+j*width;
            while(maskVert[id]!=11 && middle<end){ //找到最近的有效点
                middle+=1;
                id=middle+j*width;
            }
            if(middle==end)
                end=(start+end+1)/2;
            else{
                id=middle+width*j;

                double fa=0; //8邻域的差值的绝对值的平均
                double ph=0; //8邻域的差值的平均
                int count=0;

                //8邻域
                for(int ii=-1;ii<=1;ii++){
                    for(int jj=-1;jj<=1;jj++){
                        int idx=id+ii*width+jj;
                        int idox=ido+ii*width+jj;
                        if (maskVert[idx]==11 && maskVertN[idox]==11){
                            fa+=fabs(phaseVert[idx]-phaseVertN[idox]); //加和
                            ph+=(phaseVert[idx]-phaseVertN[idox]);
                            count+=1;
                        }
                    }
                }
                if(count!=0){ //平均
                    fa/=count;
                    ph/=count;
                }
                else{
                    fa=fabs(phaseVert[id]-vp); //vp==phaseVertN[ido]
                    ph=phaseVert[id]-vp;
                }

                if(fa<min){
                    min=fa;
                    vertitemp[j]=middle; //更新候选列索引
                }
                if(ph>=0) //目标像素在左边
                    end=middle;
                else //目标像素在右边
                    start=middle;
            }
        }
    }

    double min=255;
    w=vertitemp[(int)Pj[0]];h=Pj[0];
    for(int j=Pj[0]+1;j<Pj[1]-1;j++){
        int id=vertitemp[j]+j*width;
        if(fabs(phaseHori[id]-hp)<min && maskHori[id]==UNWRAPDONE){
            min=fabs(phaseHori[id]-hp);
            h=j; //输出行索引
            w=vertitemp[j]; //输出列索引
        }
    }

    delete [] vertitemp;
}
/*
void TwocamReconstrutor::findCorrespondence(double vp,double hp,double *phaseVert,double *phaseHori,
                                            unsigned char *maskVert,unsigned char *maskHori,
                                            int &w,int &h,Point Pi,Point Pj,int io,int jo,
                                            double *phaseVertN,unsigned char *maskVertN)
{
    double min=1E9;
    //int ido=io+jo*width;
    for(int i=Pi[0];i<Pi[1];i++){
        for(int j=Pj[0];j<Pj[1];j++){
            int id=i+j*width;
            if(maskHori[id]==UNWRAPDONE && maskVert[id]==UNWRAPDONE){
                double d=fabs(phaseHori[id]-hp)+fabs(phaseVert[id]-vp);
                if(d<min){
                    min=d;
                    w=i;h=j;
                }
            }
        }
    }
}
*/
/*
void TwocamReconstrutor::findCorrespondence(double vp,double hp,double *phaseVert,double *phaseHori,
                                            unsigned char *maskVert,unsigned char *maskHori,
                                            int &w,int &h,Point Pi,Point Pj,int io,int jo,
                                            double *phaseHoriN,unsigned char *maskHoriN)
{
    //int *vertitemp=new int[height];
    int *horitemp=new int[width];
    int ido=io+jo*width;

    //for(int j=Pj[0];j<Pj[1];j++){ //从上到下的搜索范围
    for(int i=Pi[0];i<Pi[1];i++){ //从上到下的搜索范围
        horitemp[i]=0;
        double min=255;
        //int start=Pi[0]; //从左到右的搜索范围，开始，结束
        //int end=Pi[1];
        int start=Pj[0];
        int end=Pj[1];

        while(end-start>1){ //对半查找法
            int middle=(start+end+1)/2; //中点
            //int id=middle+j*width;
            int id=i+middle*width;
            //while(maskVert[id]!=11 && middle<end){ //找到最近的有效点
            while(maskHori[id]!=11 && middle<end){
                middle+=1;
                //id=middle+j*width;
                id=i+middle*width;
            }
            if(middle==end)
                end=(start+end+1)/2;
            else{
                //id=middle+width*j;
                id=i+middle*width;

                double fa=0; //8邻域的差值的绝对值的平均
                double ph=0; //8邻域的差值的平均
                int count=0;

                //8邻域
                for(int ii=-1;ii<=1;ii++){
                    for(int jj=-1;jj<=1;jj++){
                        int idx=id+ii*width+jj;
                        int idox=ido+ii*width+jj;
                        //if (maskVert[idx]==11 && maskVertN[idox]==11){
                        if (maskHori[idx]==11 && maskHoriN[idox]==11){
                            //fa+=fabs(phaseVert[idx]-phaseVertN[idox]); //加和
                            //ph+=(phaseVert[idx]-phaseVertN[idox]);
                            fa+=fabs(phaseHori[idx]-phaseHoriN[idox]);
                            ph+=(phaseHori[idx]-phaseHoriN[idox]);
                            count+=1;
                        }
                    }
                }
                if(count!=0){ //平均
                    fa/=count;
                    ph/=count;
                }
                else{
                    //fa=fabs(phaseVert[id]-vp); //vp==phaseVertN[ido]
                    //ph=phaseVert[id]-vp;
                    fa=fabs(phaseHori[id]-hp);
                    ph=phaseHori[id]-hp;
                }

                if(fa<min){
                    min=fa;
                    //vertitemp[j]=middle; //更新候选列索引
                    horitemp[i]=middle;
                }
                if(ph>=0) //目标像素在左边
                    end=middle;
                else //目标像素在右边
                    start=middle;
            }
        }
    }

    double min=255;
    //w=vertitemp[(int)Pj[0]];
    h=horitemp[(int)Pi[0]];
    //for(int j=Pj[0]+1;j<Pj[1]-1;j++){
    for(int i=Pi[0]+1;i<Pi[1]-1;i++){
        //int id=vertitemp[j]+j*width;
        int id=i+horitemp[i]*width;
        //if(fabs(phaseHori[id]-hp)<min && maskHori[id]==UNWRAPDONE){
        if(fabs(phaseVert[id]-vp)<min && maskVert[id]==UNWRAPDONE){
            //min=fabs(phaseHori[id]-hp);
            min=fabs(phaseVert[id]-vp);
            //h=j; //输出行索引
            w=i;
            //w=vertitemp[j]; //输出列索引
            h=horitemp[i];
        }
    }

    //delete [] vertitemp;
    delete [] horitemp;
}
*/
