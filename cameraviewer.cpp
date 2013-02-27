#include "cameraviewer.h"
#include "ui_cameraviewer.h"

CameraViewer::CameraViewer(QWidget *parent,unsigned int width,unsigned int height) :
    QMainWindow(parent),imgWidth(width),imgHeight(height),
    ui(new Ui::CameraViewer)
{
    ui->setupUi(this);

    resize(imgWidth*2+60,imgHeight+200);

    frmL=new unsigned char[imgWidth*imgHeight];
    frmR=new unsigned char[imgWidth*imgHeight];

    isSave=false;
}

CameraViewer::~CameraViewer()
{
    delete ui;

    delete [] frmL;
    delete [] frmR;
}

void CameraViewer::paintEvent(QPaintEvent *)
{
    QImage imgL(imgWidth,imgHeight,QImage::Format_RGB32);
    QImage imgR(imgWidth,imgHeight,QImage::Format_RGB32);
    QPainter painter(this);
    for(unsigned int i=0;i<imgHeight;i++)
        for(unsigned int j=0;j<imgWidth;j++){
            *((QRgb *)imgL.scanLine(i)+j)=qRgb(frmL[i*imgWidth+j],frmL[i*imgWidth+j],frmL[i*imgWidth+j]);
            *((QRgb *)imgR.scanLine(i)+j)=qRgb(frmR[i*imgWidth+j],frmR[i*imgWidth+j],frmR[i*imgWidth+j]);
        }
    painter.drawImage(20,180,imgL);
    painter.drawImage(20+imgWidth+20,180,imgR);

    if(isSave){
        imgL.save(fileNameL);
        imgR.save(fileNameR);
        isSave=false;
    }
}
