#ifndef CAMERAVIEWER_H
#define CAMERAVIEWER_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QScrollArea>
#include <QTimer>

#include "globals.h"
#include "camera.h"

namespace Ui {
class CameraViewer;
}

class CameraViewer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CameraViewer(QWidget *parent = 0, ::Camera *cam = 0,
                          unsigned int imgWidth = imageWidth, unsigned int imgHeight = imageHeight);
    ~CameraViewer();

    void paintEvent(QPaintEvent *);
    
private slots:
    void grabFrames();
    void startCameraTimer();
    void stopCameraTimer();
    void snapshot(int spinIndex = -1);

    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Ui::CameraViewer *ui;

    unsigned int imgWidth, imgHeight;

public:
    unsigned char *frmL, *frmR;

    bool isSave;
    QString fileNameL;
    QString fileNameR;

    ::Camera *camera;

    QHBoxLayout *layout;
    QHBoxLayout *layout0;
    QHBoxLayout *layout1;

    QTimer *cameraTimer;
    QPushButton *startButn;
    QPushButton *stopButn;
    QSpinBox *snapshotSpin;
    QPushButton *snapshotButn;

    QLabel *label;
    QScrollArea *scrollArea0;
    QScrollArea *scrollArea1;
    QLabel *imageLabel0;
    QLabel *imageLabel1;
};

#endif // CAMERAVIEWER_H
