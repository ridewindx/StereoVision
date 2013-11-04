#include "cameraviewer.h"
#include "ui_cameraviewer.h"

#include <QDesktopWidget>
#include <QMessageBox>

CameraViewer::CameraViewer(QWidget *parent, Camera *cam, unsigned int width, unsigned int height) :
    QMainWindow(parent), camera(cam), imgWidth(width), imgHeight(height),
    ui(new Ui::CameraViewer)
{
    ui->setupUi(this);

    setWindowTitle("Camera Viewer");
    //resize(imgWidth*2 + 60, imgHeight + 200);

    frmL = new unsigned char[imgWidth * imgHeight];
    frmR = new unsigned char[imgWidth * imgHeight];

    isSave=false;

    QDesktopWidget *desktop = QApplication::desktop();
    QRect geometry = desktop->availableGeometry(desktop->primaryScreen());
    resize(geometry.width()-100, geometry.height()-100);

    QVBoxLayout *layout = new QVBoxLayout();
    ui->centralwidget->setLayout(layout);
    layout0 = new QHBoxLayout();
    layout1 = new QHBoxLayout();
    layout->addLayout(layout0);
    layout->addLayout(layout1);

    startButn = new QPushButton("Start");
    startButn->setEnabled(false);
    stopButn = new QPushButton("Stop");
    QLabel *snapshotLabel = new QLabel("Snapshot index:");
    snapshotSpin = new QSpinBox;
    snapshotSpin->setMinimum(0);
    snapshotSpin->setMaximum(100);
    snapshotButn = new QPushButton("Snapshot");
    layout0->addWidget(startButn);
    layout0->addWidget(stopButn);
    layout0->addWidget(snapshotLabel);
    layout0->addWidget(snapshotSpin);
    layout0->addWidget(snapshotButn);
    layout0->addStretch();

    label = new QLabel();
    layout1->addWidget(label);
    QHBoxLayout *dialogLayout = new QHBoxLayout();
    label->setLayout(dialogLayout);

    scrollArea0 = new QScrollArea;
    scrollArea1 = new QScrollArea;
    dialogLayout->addWidget(scrollArea0);
    dialogLayout->addWidget(scrollArea1);
    //layout1->addWidget(scrollArea0);
    //layout1->addWidget(scrollArea1);
    imageLabel0 = new QLabel;
    imageLabel1 = new QLabel;
    imageLabel0->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel0->setScaledContents(true);
    imageLabel1->setScaledContents(true);
    //imageLabel0->resize((geometry.width()-100)/2, geometry.height()-200);
    //imageLabel1->resize((geometry.width()-100)/2, geometry.height()-200);
    scrollArea0->setWidget(imageLabel0);
    scrollArea1->setWidget(imageLabel1);
    scrollArea0->setBackgroundRole(QPalette::Dark);
    scrollArea1->setBackgroundRole(QPalette::Dark);
    scrollArea0->setWidgetResizable(true);
    scrollArea1->setWidgetResizable(true);

    cameraTimer = new QTimer(this);
    connect(cameraTimer, SIGNAL(timeout()), this, SLOT(grabFrames()));
    cameraTimer->start(800); // 200

    connect(startButn, SIGNAL(clicked()), this, SLOT(startCameraTimer()));
    connect(stopButn, SIGNAL(clicked()), this, SLOT(stopCameraTimer()));
    connect(snapshotSpin, SIGNAL(valueChanged(int)), this, SLOT(snapshot(int)));
    connect(snapshotButn, SIGNAL(clicked()), this, SLOT(snapshot()));
}

CameraViewer::~CameraViewer()
{
    delete ui;

    delete [] frmL;
    delete [] frmR;
}

void CameraViewer::paintEvent(QPaintEvent *)
{
    /*
    QImage imgL(imgWidth, imgHeight, QImage::Format_RGB32);
    QImage imgR(imgWidth, imgHeight, QImage::Format_RGB32);
    QPainter painter(this);
    for (unsigned int i = 0; i < imgHeight; i++)
        for (unsigned int j = 0;j < imgWidth; j++) {
            *((QRgb *)imgL.scanLine(i)+j) = qRgb(frmL[i*imgWidth+j],frmL[i*imgWidth+j],frmL[i*imgWidth+j]);
            *((QRgb *)imgR.scanLine(i)+j) = qRgb(frmR[i*imgWidth+j],frmR[i*imgWidth+j],frmR[i*imgWidth+j]);
        }
    painter.drawImage(20, 180, imgL);
    painter.drawImage(20+imgWidth+20, 180, imgR);

    if (isSave) {
        imgL.save(fileNameL);
        imgR.save(fileNameR);
        isSave = false;
    }
    */
}

void CameraViewer::grabFrames()
{
    //camera->grabMultiFrames(frmL,3,0);
    //camera->grabMultiFrames(frmR,3,1);
    camera->grabSingleFrame(frmL, 0);
    camera->grabSingleFrame(frmR, 1);

    //update();

    QImage imgL(imgWidth, imgHeight, QImage::Format_RGB32);
    QImage imgR(imgWidth, imgHeight, QImage::Format_RGB32);
    for (unsigned int i = 0; i < imgHeight; i++)
        for (unsigned int j = 0;j < imgWidth; j++) {
            unsigned char il = frmL[i*imgWidth+j];
            unsigned char ir = frmR[i*imgWidth+j];
            *((QRgb *)imgL.scanLine(i)+j) = qRgb(il, il, il);
            *((QRgb *)imgR.scanLine(i)+j) = qRgb(ir, ir, ir);
        }

    if (isSave) {
        imgL.save(fileNameL);
        imgR.save(fileNameR);
        isSave = false;
    }

    imageLabel0->setPixmap(QPixmap::fromImage(imgL));
    imageLabel1->setPixmap(QPixmap::fromImage(imgR));
}

void CameraViewer::startCameraTimer()
{
    startButn->setEnabled(false);
    stopButn->setEnabled(true);
    cameraTimer->start();
}

void CameraViewer::stopCameraTimer()
{
    startButn->setEnabled(true);
    stopButn->setEnabled(false);
    cameraTimer->stop();
}

void CameraViewer::snapshot(int spinIndex)
{
    static int index = 0;
    if (spinIndex == -1) {
        QDir dir(workDirectory); //进入工作目录
        if (!dir.exists() || workDirectory == "") {
            QMessageBox::warning(this,tr("Entering Workspace error!"),tr("No workspace has been opened!"));
            return;
        }
        dir.mkdir("SnapshotImages");
        dir.cd(QString("SnapshotImages"));
        fileNameL = dir.absoluteFilePath(QString("L")+QString().setNum(index)+".jpg");
        fileNameR = dir.absoluteFilePath(QString("R")+QString().setNum(index)+".jpg");
        isSave = true;
        index++;
        snapshotSpin->setValue(index);
    }
    else
        index = spinIndex;
}

void CameraViewer::closeEvent(QCloseEvent *event)
{
    cameraTimer->stop();
    event->accept();
}

void CameraViewer::resizeEvent(QResizeEvent *event)
{/*
    QSize size = label->size();
    printf("\n%d, %d\n", size.width(),size.height());
    if (size.isValid()) {
        if (2*imgWidth/imgHeight < size.width()/size.height()) {
            scrollArea0->resize(imgWidth/imgHeight*size.height(), size.height());
            scrollArea1->resize(imgWidth/imgHeight*size.height(), size.height());
        } else {
            scrollArea0->resize(size.width(), imgHeight/imgWidth*size.width());
            scrollArea1->resize(size.width(), imgHeight/imgWidth*size.width());
        }
    }
*/
    QMainWindow::resizeEvent(event);
}
