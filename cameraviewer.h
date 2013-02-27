#ifndef CAMERAVIEWER_H
#define CAMERAVIEWER_H

#include <QMainWindow>

#include "globals.h"

namespace Ui {
class CameraViewer;
}

class CameraViewer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CameraViewer(QWidget *parent = 0,unsigned int imgWidth=imageWidth,unsigned int imgHeight=imageHeight);
    ~CameraViewer();

    void paintEvent(QPaintEvent *);
    
private:
    Ui::CameraViewer *ui;

    unsigned int imgWidth,imgHeight;

public:
    unsigned char *frmL,*frmR;

    bool isSave;
    QString fileNameL;
    QString fileNameR;
};

#endif // CAMERAVIEWER_H
