#include "imageviewer.h"
#include "ui_imageviewer.h"

#include <QtGUI>
#include "globals.h"
#include "imageIO.h"
#include "imageshow.h"


ImageViewer::ImageViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageViewer)
{
    ui->setupUi(this);

    QDir dir(workDirectory); //进入工作目录
    if(dir.exists(QString("NineImages"))){
        ui->lineEdit->setText(dir.absoluteFilePath(QString("NineImages")));
    }

    ui->radio_camL->setChecked(true);
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::on_Btn_hf0_clicked()
{
    selectImage("hf0");
}

void ImageViewer::on_Btn_hf1_clicked()
{
    selectImage("hf1");
}

void ImageViewer::on_Btn_hf2_clicked()
{
    selectImage("hf2");
}

void ImageViewer::on_Btn_vf0_clicked()
{
    selectImage("vf0");
}

void ImageViewer::on_Btn_vf1_clicked()
{
    selectImage("vf1");
}

void ImageViewer::on_Btn_vf2_clicked()
{
    selectImage("vf2");
}

void ImageViewer::on_Btn_hc_clicked()
{
    selectImage("hc");
}

void ImageViewer::on_Btn_vc_clicked()
{
    selectImage("vc");
}

void ImageViewer::on_Btn_bk_clicked()
{
    selectImage("bk");
}

void ImageViewer::on_Btn_browse_clicked()
{
    QDir dir(workDirectory); //进入工作目录
    QString selectedDirName=QFileDialog::getExistingDirectory(this,tr("Select Image Directories"),dir.absolutePath());
    if(selectedDirName!=""){
        ui->lineEdit->setText(selectedDirName);
    }
}

void ImageViewer::selectImage(QString imageType)
{
    QString fileName=ui->lineEdit->text();
    if(ui->lineEdit->text()!=""){
        if(ui->radio_camL->isChecked()){
            fileName.append("/CamL"+imageType+".raw");
        }
        else if(ui->radio_camR->isChecked()){
            fileName.append("/CamR"+imageType+".raw");
        }
        /*
        unsigned char *image=new unsigned char[imageWidth*imageHeight];
        if(!readBytesFromFile(fileName.toStdString(),image,imageWidth*imageHeight)){
            QMessageBox::warning(this,tr("Open Images: Error!"),tr(("Image "+fileName+" doesn't exist!").toStdString().c_str()));
        }
        emit showImage(image);
        */
        ImageShow imageShow;
        imageShow.init(fileName,imageWidth,imageHeight);
        imageShow.setGeometry(0,0,imageWidth,imageHeight);
        imageShow.exec();
    }
    else{
        QMessageBox::warning(this,tr("Open Images: Error!"),tr("Please Add Path!"));
    }
}
