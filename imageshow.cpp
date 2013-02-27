#include "imageshow.h"
#include "ui_imageshow.h"

#include "imageIO.h"

#include <QPainter>
#include <iostream>

ImageShow::ImageShow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageShow)
{
    ui->setupUi(this);

    image=NULL;
}

ImageShow::~ImageShow()
{
    delete ui;
}

void ImageShow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    int intensity;
    if(imageName.right(3)==tr("raw")){
        image=new unsigned char[size];
        readBytesFromFile(imageName.toStdString(),image,size);
        for(int i=0;i<width;i++)
            for(int j=0;j<height;j++){
                intensity=image[i+width*j];
                pen.setColor(QColor(intensity,intensity,intensity));
                painter.setPen(pen);
                painter.drawPoint(i,j);
            }
        delete [] image;
        image=NULL;
    }
    else if(imageName.right(3)==tr("bmp")){
        QPixmap pixmap;
        pixmap.load(imageName);
        painter.drawPixmap(0,0,width,height,pixmap);
    }
    else if(imageName==tr("directshow") && image!=NULL){
        for(int i=0;i<width;i++)
            for(int j=0;j<height;j++){
                intensity=image[i+width*j];
                pen.setColor(QColor(intensity,intensity,intensity));
                painter.setPen(pen);
                painter.drawPoint(i,j);
            }
    }
}
