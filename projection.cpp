#include "projection.h"
#include "ui_projection.h"

#include <QPainter>
#include <QLabel>
#define _USE_MATH_DEFINES
#include <cmath>
#include <xutility>
#include "globals.h"

/*
int projectionX=-1024;
int projectionY=0;
int projectionWidth=1024;
int projectionHeight=768;
*/

int projectionX=1440;
int projectionY=0;
//int projectionX=350;
//int projectionY=250;
//int projectionWidth=480;
//int projectionHeight=320;
//int projectionWidth=640;
//int projectionHeight=480;
int projectionWidth=608;
int projectionHeight=684;

Projection::Projection(QWidget *parent, bool isInteractive) :
    QDialog(parent),
    ui(new Ui::Projection)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);
    resize(projectionWidth, projectionHeight);
    setGeometry(projectionX, projectionY, projectionWidth, projectionHeight);
    setCursor(Qt::BlankCursor);
    nBits = ceil(log(double(projectionWidth > projectionHeight ? projectionWidth : projectionHeight)) / M_LN2);
    maxCode = ceil(pow(2.0, nBits));
    offsetH = (maxCode - projectionHeight) / 2;
    offsetV = (maxCode - projectionWidth) / 2;
    hCode = new unsigned int[maxCode];
    vCode = new unsigned int[maxCode];
    generateGrayCode();

    controlDialog = new QDialog(parent);
    controlDialog->setWindowTitle("Projection Control");
    controlDialog->setGeometry(50, 50, 180, 60);
    QSlider *controlSlider = new QSlider(Qt::Horizontal, controlDialog);
    controlSlider->setGeometry(20, 20, 110, 20);
    controlSlider->setMinimum(0);
    controlSlider->setMaximum(50);
    controlSlider->setSliderPosition(0);
    connect(controlSlider, SIGNAL(valueChanged(int)), this, SLOT(setFringeType(int)));
    QLabel *controlLabel = new QLabel(controlDialog);
    controlLabel->setGeometry(150, 20, 20, 20);
    connect(controlSlider, SIGNAL(valueChanged(int)), controlLabel, SLOT(setNum(int)));
    controlSlider->setValue(0);
    controlLabel->setNum(0);
    if(isInteractive)
        controlDialog->show();
}

Projection::~Projection()
{
    delete ui;

    delete [] hCode;
    delete [] vCode;
}

void Projection::generateGrayCode()
{
    for (unsigned int i = 0; i < maxCode; i++) {
        hCode[i] = vCode[i] = (i>>1) ^ i; // Binary Code to Gray Code
    }
}

void Projection::setFringeType(int value)
{
    fringeType=value;
    update();
}

void Projection::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    double r=0, g=0, b=0;
    double color=0;
    double T=30; // period in pixels
    switch(fringeType){
    case HF0:
        for(int j=0;j<projectionHeight;j++){
            color=cos(j*2*M_PI/T)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);

        }
        break;
    case HF1:
        for(int j=0;j<projectionHeight;j++){
            color=cos(j*2*M_PI/T+M_PI*2/3)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);

        }
        break;
    case HF2:
        for(int j=0;j<projectionHeight;j++){
            color=cos(j*2*M_PI/T+M_PI*4/3)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);

        }
        break;
    case VF0:
        for(int i=0;i<projectionWidth;i++){
            color=cos(i*2*M_PI/T)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case VF1:
        for(int i=0;i<projectionWidth;i++){
            color=cos(i*2*M_PI/T+2*M_PI/3)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case VF2:
        for(int i=0;i<projectionWidth;i++){
            color=cos(i*2*M_PI/T+4*M_PI/3)+1;
            color=color*255/2;
            pen.setColor(QColor(color,color,color));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case HC:
        for(int j=0;j<projectionHeight;j++){
            if(j==projectionHeight/2)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case VC:
        for(int i=0;i<projectionWidth;i++){
            if(i==projectionWidth/2)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case BK: //Full Black
        for(int i=0;i<projectionWidth;i++){
            pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case 109: //RGB Horizontal Phase Shifting
        for(int j=0;j<projectionHeight;j++){
            r=( cos(j*2*M_PI/T)+1 )*255/2;
            g=( cos(j*2*M_PI/T+2*M_PI/3)+1 )*255/2;
            b=( cos(j*2*M_PI/T+4*M_PI/3)+1 )*255/2;
            pen.setColor(QColor(r,g,b));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case 110: //RGB Vertical Phase Shifting
        for(int i=0;i<projectionWidth;i++){
            r=( cos(i*2*M_PI/T)+1 )*255/2;
            g=( cos(i*2*M_PI/T+2*M_PI/3)+1 )*255/2;
            b=( cos(i*2*M_PI/T+4*M_PI/3)+1 )*255/2;
            pen.setColor(QColor(r,g,b));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case WT: //Full White
        for(int i=0;i<projectionWidth;i++){
            pen.setColor(QColor(255,255,255));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    /*** Gray Code, Horizontal Stripes, From Bottom to Top ***/
    case GCH0: //The most-significant bit
        for(int j=0;j<projectionHeight;j++){
            /*if(j<projectionHeight/2)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));*/
            //bool wordBit=(hCode[j+96]>>8)%2; //Centralization, (512-320)/2=96
            bool wordBit=(hCode[j+offsetH]>>(nBits-1))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH1:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>7)%2; //Centralization, (512-320)/2=96
            bool wordBit=(hCode[j+offsetH]>>(nBits-2))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH2:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>6)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-3))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH3:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>5)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-4))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH4:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>4)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-5))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH5:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>3)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-6))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH6:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>2)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-7))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH7:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>1)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-8))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH8:
        for(int j=0;j<projectionHeight;j++){
            //bool wordBit=(hCode[j+96]>>0)%2;
            bool wordBit=(hCode[j+offsetH]>>(nBits-9))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    case GCH9:
        for(int j=0;j<projectionHeight;j++){
            bool wordBit=(hCode[j+offsetH]>>(nBits-10))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(0,j,projectionWidth,j);
        }
        break;
    /*** Gray Code, Horizontal Stripes, From Bottom to Top ***/
    case GCV0: //The most-significant bit
        for(int i=0;i<projectionWidth;i++){
            /*if(i<projectionWidth/2)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));*/
            //bool wordBit=(vCode[i+16]>>8)%2; //Centralization, (512-480)/2=16
            bool wordBit=(vCode[i+offsetV]>>(nBits-1))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV1:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>7)%2; //Centralization, (512-480)/2=16
            bool wordBit=(vCode[i+offsetV]>>(nBits-2))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV2:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>6)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-3))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV3:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>5)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-4))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV4:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>4)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-5))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV5:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>3)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-6))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV6:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>2)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-7))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV7:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>1)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-8))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV8:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>0)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-9))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    case GCV9:
        for(int i=0;i<projectionWidth;i++){
            //bool wordBit=(vCode[i+16]>>0)%2;
            bool wordBit=(vCode[i+offsetV]>>(nBits-10))%2;
            if(wordBit)
                pen.setColor(QColor(255,255,255));
            else
                pen.setColor(QColor(0,0,0));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    default: //Full White
        for(int i=0;i<projectionWidth;i++){
            pen.setColor(QColor(255,255,255));
            painter.setPen(pen);
            painter.drawLine(i,0,i,projectionHeight);
        }
        break;
    }
}

void Projection::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    controlDialog->hide();
}
