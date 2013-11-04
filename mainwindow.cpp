#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QSlider>

#include "imageviewer.h"
#include <fstream>
#include "imageshow.h"
#include "mat3.h"
#include "calib.h"
#include "features.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    nineImagesMaker(imageWidth,imageHeight),
    nineGrayCodeMaker(imageWidth, imageHeight, projectionWidth, projectionHeight)
{
    ui->setupUi(this);

    camera=0;
//    frmL=0;
//    frmR=0;
    frmL=new unsigned char[3*imageWidth*imageHeight];
    frmR=new unsigned char[3*imageWidth*imageHeight];
    cameraViewer=0;
    /*cameraTimer=0;
    startButn=0;
    stopButn=0;*/

    projection=new Projection(this);
    projection->hide();

    pcTimer=0;

    modelShow=0;
}

MainWindow::~MainWindow()
{
    delete ui;

    delete projection;

    if(modelShow){
        delete modelShow;
    }

    if (camera) {
        camera->close();
        delete camera;
    }
}

void MainWindow::projectFringes(FringeType fringeType)
{
    projection->fringeType=fringeType;
    projection->update();
    projection->show();
}

void MainWindow::on_actionHorizontal_1_triggered()
{
    projectFringes(HF0);
}

void MainWindow::on_actionHorizontal_2_triggered()
{
    projectFringes(HF1);
}

void MainWindow::on_actionHorizontal_3_triggered()
{
    projectFringes(HF2);
}

void MainWindow::on_actionVertical_0_triggered()
{
    projectFringes(VF0);
}

void MainWindow::on_actionVertical_1_triggered()
{
    projectFringes(VF1);
}

void MainWindow::on_actionVertical_2_triggered()
{
    projectFringes(VF2);
}

void MainWindow::on_actionHorizontal_Center_Line_triggered()
{
    projectFringes(HC);
}

void MainWindow::on_actionVertical_Center_Line_triggered()
{
    projectFringes(VC);
}

void MainWindow::on_actionFull_Black_triggered()
{
    projectFringes(BK);
}



void MainWindow::on_actionNine_Images_Maker_triggered()
{
    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists() || workDirectory==""){
        QMessageBox::warning(this,"Entering Workspace error!","No workspace has been opened!\n"
                             "Please create a new workspace or open an existing workspace.");
        return; //Nothing happen
    }
    dir.mkdir("NineImages"); //建立"NineImages"目录, 或者已经存在
    //QString dirSelected=QFileDialog::getExistingDirectory(this,"Select Directory with Nine*2 Raw Images",dir.absolutePath()); //选择目录对话框
    //if(!dirSelected.isEmpty()){ //选择的目录
    //    dir.setPath(dirSelected);
    //}
    //else{ //若点击“取消”按钮，则选择默认目录
        dir.cd(QString("NineImages"));
    //}

    if(!nineImagesMaker.initialMaker(dir)){
        QMessageBox::warning(this,"Reading images error!","The directory \""+dir.absolutePath()+"\" doesn't have nine*2 specified images!"
                             "Please check if they have correct filenames.");
        return;
    }

    dir.cdUp(); // 返回上一级目录
    QString image3DName("NineImagesModel.md");
    //image3DName=QFileDialog::getSaveFileName(this,"Save NineImagesModel File",
    //                            dir.absoluteFilePath(image3DName),"Model (*.md)");
    //if(image3DName.isEmpty()){ //若点击“取消”按钮，则选择默认文件名
        image3DName=dir.absoluteFilePath(QString("NineImagesModel.md"));
    //}
    //if(image3DName.lastIndexOf(QString(".md"))==-1){ //若无后缀名，则加上
    //    image3DName.append(".md");
    //}
    if(!nineImagesMaker.make3Dmodel(image3DName.toStdString())){
        QMessageBox::warning(this,"Makeing 3D model error!","Failed in makeing 3D model !");
        return;
    }
    QMessageBox::information(this,"Object model making succeeds!","Object model information have been output to file:\n\""+image3DName+"\".");
}



void MainWindow::on_actionObject_Coordinates_Output_triggered()
{
    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists() || workDirectory==""){
        QMessageBox::warning(this,tr("Entering Workspace error!"),tr("No workspace has been opened!"));
        return;
    }
    QString image3DName=dir.absoluteFilePath(QString("NineImagesModel.md"));
    if(image3DName==""){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("No file \"NineImagesModel.md\" found!"));
        return;
    }
    IMAGE3D *image=new IMAGE3D[imageSize()];
    if(!readSingleImage3D(image3DName.toStdString(),image)){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("Reading file \"NineImagesModel.md\" failed!"));
        return;
    }
    /*
    QString outFileName("ObjectCoors.txt");
    if(!writeImage3DCoordinates(dir.absoluteFilePath(outFileName).toStdString(),image,imageWidth,imageHeight)){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("Writing file \"ObjectCoors.txt\" failed!"));
        return;
    }
    */

    QString pcdFileName("ObjectCoors.pcd");
    if(!writeToPCD(dir.absoluteFilePath(pcdFileName).toStdString(),image,imageWidth,imageHeight)){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("Writing file \"ObjectCoors.pcd\" failed!"));
        return;
    }

    delete [] image;
    QMessageBox::information(this,tr("Object coordinates output succeeds!"),tr("Object coordinates have been output to file \"ObjectCoors.txt\" and \"ObjectCoors.pcd\"."));
}



void MainWindow::on_actionNine_Images_Show_triggered()
{
    ImageViewer imageViewer;
    imageViewer.exec();
}




void MainWindow::on_actionNew_Workspace_triggered()
{
    QDir dir = QDir::currentPath(); //返回当前目录
    QString directory = QFileDialog::getSaveFileName(this,
                                                         tr("Select directory and input name to create a new Workspace"),
                                                         dir.absolutePath());
    if (!directory.isEmpty()) { //选择的目录
        dir.mkdir(directory);
        dir.cd(directory);
        /*if(dir.exists("Workspace.sv")){
            QMessageBox::warning(this,tr("Create Workspace error!"),tr("Workspace has exist in this directory!"));
            return;
        }*/
        std::ofstream workspaceFile;
        workspaceFile.open((directory+"/Workspace.sv").toStdString().c_str());
        workspaceFile.close();
        dir.mkdir("NineImages");
        dir.mkdir("TwoCamCalibData");
        workDirectory = directory;
        setWindowTitle("Stereo Vision: Workspace \""+workDirectory+"\"");
    } else { //若点击“取消”按钮
        //QMessageBox::warning(this,tr("Create Workspace error!"),tr("No new workspace name specified!"));
        return;
    }
}

void MainWindow::on_actionOpen_Workspace_triggered()
{
    QDir dir = QDir::currentPath(); //返回当前目录
    QString directory = QFileDialog::getExistingDirectory(this,
                                                              tr("Open a Workspace directory"),
                                                              dir.absolutePath()); //选择目录对话框
    if (!directory.isEmpty()) { //选择的目录
        dir.cd(directory);
        if(!dir.exists("Workspace.sv")){
            QMessageBox::warning(this,
                                 tr("Open Workspace error!"),
                                 tr("Workspace hasn\'t exist in this directory!"));
            return;
        }
        workDirectory = directory;
        setWindowTitle("Stereo Vision: Workspace \""+workDirectory+"\"");
    } else { //若点击“取消”按钮
        //QMessageBox::warning(this, tr("Open Workspace error!"), tr("No existing workspace name specified!"));
        return;
    }

    calib::test();
    calib::test2();

    //features::testHarrisCorner();
}



void MainWindow::on_actionLeft_Horizontal_triggered()
{
    phaseMapShow(0);
}

void MainWindow::on_actionLeft_Vertical_triggered()
{
    phaseMapShow(1);
}

void MainWindow::on_actionRight_Horizontal_triggered()
{
    phaseMapShow(2);
}

void MainWindow::on_actionRight_Vertical_triggered()
{
    phaseMapShow(3);
}

void MainWindow::phaseMapShow(int phaseNo)
{
    NineImagesMaker &maker = nineGrayCodeMaker;

    double max, min;
    maker.getPhaseMaxMin(max, min);
    double Mm = max - min;
    int base = 50;
    int width = maker.getWidth();
    int height = maker.getHeight();
    int size = maker.getSize();
    double *phase = maker.phase + size * phaseNo;
    unsigned char *mask = maker.mask + size * phaseNo;
    unsigned char *mapImage = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        if (mask[i] == UNWRAPDONE) {
            mapImage[i] = base + (phase[i]-min) / Mm * (255-base);
        } else
            mapImage[i]=0;
    }
    ImageShow imageShow;
    imageShow.init(tr("directshow"), width, height, mapImage);
    imageShow.setGeometry(0, 0, width, height);
    imageShow.exec();
}



void MainWindow::on_actionOutput_ALL_Four_triggered()
{
    NineImagesMaker &maker = nineGrayCodeMaker;

    int width=maker.getWidth();
    int height=maker.getHeight();
    int size=maker.getSize();

    for (int phaseNo = 0; phaseNo < 4; phaseNo++) {
        double *phase = maker.phase + size*phaseNo;
        unsigned char *mask = maker.mask + size*phaseNo;
        QString fileName;
        switch (phaseNo) {
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
        for (int i = 0; i < height; i++) {
            for (int j = 0;j < width; j++){
                if (mask[i*width + j] == UNWRAPDONE)
                    outFile << i+1 << " " << j+1 << " " << phase[i*width+j] << endl;
                    //outFile << phase[i*width+j] << " ";
                //else
                    //outFile << 0 << " ";
            }
            //outFile<<std::endl;
        }
        outFile.close();
    }
}

void MainWindow::on_actionRestructed_Model_Show_triggered()
{
    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists()){
        QMessageBox::warning(this,tr("Entering Workspace error!"),tr("No workspace has been opened!"));
        return;
    }
    QString image3DName=dir.absoluteFilePath(QString("NineImagesModel.md"));
    if(image3DName==""){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("No file \"NineImagesModel.md\" found!"));
        return;
    }
    IMAGE3D *image=new IMAGE3D[imageSize()];
    if(!readSingleImage3D(image3DName.toStdString(),image)){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("Reading file \"NineImagesModel.md\" failed!"));
        return;
    }
/*
    QString nameCoorNorm=dir.absoluteFilePath(QString("ObjectCoorsNormals.pcd"));
    if(nameCoorNorm==""){
        QMessageBox::warning(this,tr("Object coordinates and normals reading error!"),tr("No file \"ObjectCoorsNormals.pcd\" found!"));
        return;
    }
    if(!readFromPCD(image,imageWidth,imageHeight,nameCoorNorm.toStdString())){
        QMessageBox::warning(this,tr("Object coordinates and normals reading error!"),tr("Reading file \"ObjectCoorsNormals.pcd\" failed!"));
        return;
    }
    QString outFileName("ObjectCoorsNormals.txt");
    if(!writeImage3DCoordinates(dir.absoluteFilePath(outFileName).toStdString(),image,imageWidth,imageHeight)){
        QMessageBox::warning(this,tr("Object coordinates output error!"),tr("Writing file \"ObjectCoorsNormals.txt\" failed!"));
        return;
    }
*/
    /*
    Point TT(5.9049,0.1402,8.3733);
    double m[3][3]={{0.8636,0.0067,0.5042},{-0.0082,0.999999,0.0008},{-0.5042,-0.0048,0.8636}};
    Mat3 RR(m);
    for(unsigned int i=0;i<imageSize();i++){
        image[i].coor=RR*image[i].coor+TT;
    }
    */
    if(modelShow==0){
        modelShow=new ModelShow(imageWidth,imageHeight);
    }
    modelShow->setImage(image);
    QDesktopWidget* desktopWidget = QApplication::desktop();
    //获取屏幕可用大小
    QRect deskRect = desktopWidget->availableGeometry();
    //获取屏幕实际大小
    //QRect screenRect = desktopWidget->screenGeometry();
    int availX=modelShow->frameGeometry().width()-modelShow->geometry().width();
    int availY=modelShow->frameGeometry().height()-modelShow->geometry().height();
    modelShow->setGeometry(availX-availX/2,availY-availX/2,deskRect.width()-availX,deskRect.height()-availY);
    //modelShow->setFixedSize(deskRect.width()-availX,deskRect.height()-availY);
    modelShow->drawModel();
    modelShow->show();
    delete [] image;
}



void MainWindow::on_actionOpen_Cameras_triggered()
{
    if (camera) {
        //close cameras
        camera->close();
        delete camera;
        camera=0;
    }
#ifdef HAS_POINT_GREY
    camera = new CameraPointGrey(imageWidth,imageHeight);
#endif
    if (camera->init(false) == -1) {
        QMessageBox::warning(this,"Open Cameras Error", camera->getErrorString().c_str());
        camera->close();
        delete camera;
        camera=0;
    }
}

void MainWindow::on_actionOpen_Triggered_Cameras_triggered()
{
    if (camera) {
        //close cameras
        camera->close();
        delete camera;
        camera=0;
    }
#ifdef HAS_POINT_GREY
    camera = new CameraPointGrey(imageWidth,imageHeight);
#endif
    if (camera->init(true) == -1) {
        QMessageBox::warning(this,"Open Triggered Cameras Error", camera->getErrorString().c_str());
        camera->close();
        delete camera;
        camera=0;
    }
}

void MainWindow::on_actionClose_Cameras_triggered()
{
    if (camera) {
        camera->close();
        delete camera;
        camera=0;
    }
}

void MainWindow::on_actionCameras_Viewer_triggered()
{
    if (!camera)
        on_actionOpen_Cameras_triggered();

//    if(frmL)
//        delete [] frmL;
//    if(frmR)
//        delete [] frmR;
//    frmL=new unsigned char[3*imageWidth*imageHeight];
//    frmR=new unsigned char[3*imageWidth*imageHeight];

    cameraViewer = new CameraViewer(this, camera, imageWidth, imageHeight);
//    if (!cameraTimer) {
//        cameraTimer = new QTimer(this);
//        connect(cameraTimer, SIGNAL(timeout()), this, SLOT(grabFrames()));
//    }
//    cameraTimer->start(200);
//    startButn = new QPushButton("Start");
//    stopButn = new QPushButton("Stop");
//    QLabel *snapshotLabel = new QLabel("Snapshot index:");
//    snapshotSpin = new QSpinBox;
//    snapshotButn = new QPushButton("Snapshot");
//    QHBoxLayout *layout = new QHBoxLayout;
//    layout->addWidget(startButn);
//    layout->addWidget(stopButn);
//    layout->addWidget(snapshotLabel);
//    layout->addWidget(snapshotSpin);
//    layout->addWidget(snapshotButn);
//    cameraViewer->setLayout(layout);
//    //startButn->setGeometry(20,20,60,30);
//    //stopButn->setGeometry(20+60+20,20,60,30);
//    //snapshotLabel->setGeometry(20+60+20+60+20,20,80,30);
//    //snapshotSpin->setGeometry(20+60+20+60+20+80+20,20,60,30);
//    snapshotSpin->setMinimum(0);
//    snapshotSpin->setMaximum(100);
//    //snapshotButn->setGeometry(20+60+20+60+20+60+30+60+20,20,60,30);
//    startButn->setEnabled(false);
//    connect(startButn, SIGNAL(clicked()), this, SLOT(startCameraTimer()));
//    connect(stopButn, SIGNAL(clicked()), this, SLOT(stopCameraTimer()));
//    connect(snapshotSpin, SIGNAL(valueChanged(int)), this, SLOT(snapshot(int)));
//    connect(snapshotButn, SIGNAL(clicked()), this, SLOT(snapshot()));
//    cameraViewer->setWindowTitle("Camera Viewer");
    cameraViewer->show();
}



void MainWindow::on_actionProjection_Control_triggered()
{
    projection->fringeType = -1;
    projection->update();
    projection->show();
    //projection->controlDialog->findChild<QSlider *>()->setValue(0);
    projection->controlDialog->show();
    projection->setFringeType(0);
}

void MainWindow::on_actionProject_and_Capture_triggered()
{
    if (!camera)
        on_actionOpen_Cameras_triggered();

    if(pcTimer)
        delete pcTimer;
    pcTimer = new QTimer(this);
    projection->fringeType = HF0;
    projection->update();
    projection->show();
    connect(pcTimer, SIGNAL(timeout()), this, SLOT(projectAndCaptureAction()));
    pcTimer->start(800);
}

void MainWindow::projectAndCaptureAction()
{
    //camera->grabMultiFrames(frmL,3,0);
    //camera->grabMultiFrames(frmR,3,1);
    camera->grabSingleFrame(frmL, 0);
    camera->grabSingleFrame(frmR, 1);

    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists() || workDirectory==""){
        QMessageBox::warning(this,tr("Entering Workspace error!"),tr("No workspace has been opened!"));
        return;
    }
    dir.mkdir("NineImages"); //建立目录，或者已经存在
    dir.mkdir("GrayCodeImages");

    string fileNamesNineL[9]={"CamLhf0.raw","CamLhf1.raw","CamLhf2.raw","CamLvf0.raw","CamLvf1.raw","CamLvf2.raw","CamLhc.raw","CamLvc.raw","CamLbk.raw"};
    string fileNamesNineR[9]={"CamRhf0.raw","CamRhf1.raw","CamRhf2.raw","CamRvf0.raw","CamRvf1.raw","CamRvf2.raw","CamRhc.raw","CamRvc.raw","CamRbk.raw"};
    /*
    string fileNamesGCL[19]={"CamLwt.raw","CamLgch0.raw","CamLgch1.raw","CamLgch2.raw","CamLgch3.raw","CamLgch4.raw","CamLgch5.raw","CamLgch6.raw","CamLgch7.raw","CamLgch8.raw",
                                          "CamLgcv0.raw","CamLgcv1.raw","CamLgcv2.raw","CamLgcv3.raw","CamLgcv4.raw","CamLgcv5.raw","CamLgcv6.raw","CamLgcv7.raw","CamLgcv8.raw"};
    string fileNamesGCR[19]={"CamRwt.raw","CamRgch0.raw","CamRgch1.raw","CamRgch2.raw","CamRgch3.raw","CamRgch4.raw","CamRgch5.raw","CamRgch6.raw","CamRgch7.raw","CamRgch8.raw",
                                          "CamRgcv0.raw","CamRgcv1.raw","CamRgcv2.raw","CamRgcv3.raw","CamRgcv4.raw","CamRgcv5.raw","CamRgcv6.raw","CamRgcv7.raw","CamRgcv8.raw"};
    */
    string fileNameWTL = "CamLwt.raw";
    string fileNamesGCHL[] = {"CamLgch0.raw","CamLgch1.raw","CamLgch2.raw","CamLgch3.raw","CamLgch4.raw","CamLgch5.raw","CamLgch6.raw","CamLgch7.raw","CamLgch8.raw","CamLgch9.raw","CamLgch10.raw"};
    string fileNamesGCVL[] = {"CamLgcv0.raw","CamLgcv1.raw","CamLgcv2.raw","CamLgcv3.raw","CamLgcv4.raw","CamLgcv5.raw","CamLgcv6.raw","CamLgcv7.raw","CamLgcv8.raw","CamLgcv9.raw","CamLgcv10.raw"};
    string fileNameWTR = "CamRwt.raw";
    string fileNamesGCHR[] = {"CamRgch0.raw","CamRgch1.raw","CamRgch2.raw","CamRgch3.raw","CamRgch4.raw","CamRgch5.raw","CamRgch6.raw","CamRgch7.raw","CamRgch8.raw","CamRgch9.raw","CamRgch10.raw"};
    string fileNamesGCVR[] = {"CamRgcv0.raw","CamRgcv1.raw","CamRgcv2.raw","CamRgcv3.raw","CamRgcv4.raw","CamRgcv5.raw","CamRgcv6.raw","CamRgcv7.raw","CamRgcv8.raw","CamRgcv9.raw","CamRgcv10.raw"};


    QString imgName;
    switch(projection->fringeType){
    case HF0:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[0]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[0]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=HF1;
        projection->update();
        projection->show();
        break;
    case HF1:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[1]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[1]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=HF2;
        projection->update();
        projection->show();
        break;
    case HF2:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[2]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[2]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=VF0;
        projection->update();
        projection->show();
        break;
    case VF0:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[3]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[3]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=VF1;
        projection->update();
        projection->show();
        break;
    case VF1:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[4]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[4]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=VF2;
        projection->update();
        projection->show();
        break;
    case VF2:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[5]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[5]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=HC;
        projection->update();
        projection->show();
        break;
    case HC:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[6]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[6]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=VC;
        projection->update();
        projection->show();
        break;
    case VC:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[7]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[7]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=BK;
        projection->update();
        projection->show();
        break;
    case BK:
        dir.cd(QString("NineImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineL[8]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesNineR[8]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=WT;
        projection->update();
        projection->show();
        break;

    case WT:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNameWTL));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNameWTR));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH0;
        projection->update();
        projection->show();
        break;
    case GCH0:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[0]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[0]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH1;
        projection->update();
        projection->show();
        break;
    case GCH1:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[1]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[1]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH2;
        projection->update();
        projection->show();
        break;
    case GCH2:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[2]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[2]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH3;
        projection->update();
        projection->show();
        break;
    case GCH3:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[3]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[3]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH4;
        projection->update();
        projection->show();
        break;
    case GCH4:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[4]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[4]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH5;
        projection->update();
        projection->show();
        break;
    case GCH5:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[5]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[5]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH6;
        projection->update();
        projection->show();
        break;
    case GCH6:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[6]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[6]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH7;
        projection->update();
        projection->show();
        break;
    case GCH7:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[7]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[7]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH8;
        projection->update();
        projection->show();
        break;
    case GCH8:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[8]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[8]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCH9;
        projection->update();
        projection->show();
        break;
    case GCH9:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHL[9]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCHR[9]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV0;
        projection->update();
        projection->show();
        break;
    case GCV0:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[0]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[0]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV1;
        projection->update();
        projection->show();
        break;
    case GCV1:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[1]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[1]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV2;
        projection->update();
        projection->show();
        break;
    case GCV2:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[2]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[2]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV3;
        projection->update();
        projection->show();
        break;
    case GCV3:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[3]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[3]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV4;
        projection->update();
        projection->show();
        break;
    case GCV4:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[4]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[4]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV5;
        projection->update();
        projection->show();
        break;
    case GCV5:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[5]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[5]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV6;
        projection->update();
        projection->show();
        break;
    case GCV6:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[6]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[6]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV7;
        projection->update();
        projection->show();
        break;
    case GCV7:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[7]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[7]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV8;
        projection->update();
        projection->show();
        break;
    case GCV8:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[8]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[8]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=GCV9;
        projection->update();
        projection->show();
        break;
    case GCV9:
        dir.cd(QString("GrayCodeImages"));
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVL[9]));
        writeBytesToFile(imgName.toStdString(),frmL,imageWidth*imageHeight);
        imgName=dir.absoluteFilePath(QString::fromStdString(fileNamesGCVR[9]));
        writeBytesToFile(imgName.toStdString(),frmR,imageWidth*imageHeight);
        projection->fringeType=-1;
        projection->update();
        projection->hide();
        pcTimer->stop();
        break;
    default:
        QMessageBox::warning(this,QString("Project and Capture Error"),QString("Fringe type is wrong!"));
        break;
    }
}



void MainWindow::on_actionNine_Images_Average_triggered()
{
    QString suffix[]={"1","2","3","4","5"};
    QString fileNamesNineL[9]={"CamLhf0.raw","CamLhf1.raw","CamLhf2.raw","CamLvf0.raw","CamLvf1.raw","CamLvf2.raw","CamLhc.raw","CamLvc.raw","CamLbk.raw"};
    QString fileNamesNineR[9]={"CamRhf0.raw","CamRhf1.raw","CamRhf2.raw","CamRvf0.raw","CamRvf1.raw","CamRvf2.raw","CamRhc.raw","CamRvc.raw","CamRbk.raw"};
    unsigned char *imgL=new unsigned char[9*imageWidth*imageHeight];
    unsigned char *imgR=new unsigned char[9*imageWidth*imageHeight];
    int *meanL=new int[9*imageWidth*imageHeight];
    int *meanR=new int[9*imageWidth*imageHeight];
    for(unsigned int i=0;i<9*imageWidth*imageHeight;i++){
        meanL[i]=0;meanR[i]=0;
    }
    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists() || workDirectory==""){
        QMessageBox::warning(this,tr("Entering Workspace error!"),tr("No workspace has been opened!"));
        return;
    }
    for(int i=0;i<5;i++){
        dir.cd(QString("NineImages")+suffix[i]);
        for(int j=0;j<9;j++){
            readBytesFromFile(dir.absoluteFilePath(fileNamesNineL[j]).toStdString(),&imgL[j*imageWidth*imageHeight],imageWidth*imageHeight);
            readBytesFromFile(dir.absoluteFilePath(fileNamesNineR[j]).toStdString(),&imgR[j*imageWidth*imageHeight],imageWidth*imageHeight);
            for(unsigned int k=0;k<imageWidth*imageHeight;k++){
                meanL[k+j*imageWidth*imageHeight] += imgL[k+j*imageWidth*imageHeight];
                meanR[k+j*imageWidth*imageHeight] += imgR[k+j*imageWidth*imageHeight];
            }
        }
        dir.cdUp();
    }
    for(unsigned int i=0;i<9*imageWidth*imageHeight;i++){
        imgL[i]=meanL[i]/5;imgR[i]=meanR[i]/5;
    }
    dir.mkdir("NineImages");
    dir.cd("NineImages");
    for(int j=0;j<9;j++){
        writeBytesToFile(dir.absoluteFilePath(fileNamesNineL[j]).toStdString(),&imgL[j*imageWidth*imageHeight],imageWidth*imageHeight);
        writeBytesToFile(dir.absoluteFilePath(fileNamesNineR[j]).toStdString(),&imgR[j*imageWidth*imageHeight],imageWidth*imageHeight);
    }
}



void MainWindow::on_actionNine_Images_Gray_Code_Maker_triggered()
{
    QDir dir(workDirectory); //进入工作目录
    if(!dir.exists() || workDirectory==""){
        QMessageBox::warning(this,"Entering Workspace error!","No workspace has been opened!\n"
                             "Please create a new workspace or open an existing workspace.");
        return; //Nothing happen
    }
    dir.mkdir("NineImages"); //建立"NineImages"目录, 或者已经存在
    dir.cd(QString("NineImages"));
    QDir nineimagesDir = dir;
    dir.cdUp();
    dir.mkdir("GrayCodeImages");
    dir.cd(QString("GrayCodeImages"));
    QDir graycodeimagesDir = dir;

    if(!nineGrayCodeMaker.initialMaker(nineimagesDir, graycodeimagesDir)){
        QMessageBox::warning(this,"Reading images error!","The directory \""+dir.absolutePath()+"\" doesn't have nine*2 specified images!"
                             "Please check if they have correct filenames.");
        return;
    }

    dir.cdUp(); // 返回上一级目录
    QString image3DName("NineImagesModel.md");
    image3DName=dir.absoluteFilePath(QString("NineImagesModel.md"));

    if(!nineGrayCodeMaker.make3Dmodel(image3DName.toStdString())){
        QMessageBox::warning(this,"Makeing 3D model error!","Failed in makeing 3D model !");
        return;
    }
    QMessageBox::information(this,"Object model making succeeds!","Object model information have been output to file:\n\""+image3DName+"\".");
}

