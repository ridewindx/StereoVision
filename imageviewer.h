#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>

namespace Ui {
class ImageViewer;
}

class ImageViewer : public QDialog
{
    Q_OBJECT
    
public:
    explicit ImageViewer(QWidget *parent = 0);
    ~ImageViewer();
    
private slots:
    void on_Btn_hf0_clicked();
    void on_Btn_hf1_clicked();
    void on_Btn_hf2_clicked();
    void on_Btn_vf0_clicked();
    void on_Btn_vf1_clicked();
    void on_Btn_vf2_clicked();
    void on_Btn_hc_clicked();
    void on_Btn_vc_clicked();
    void on_Btn_bk_clicked();

    void on_Btn_browse_clicked();

private:
    Ui::ImageViewer *ui;

    void selectImage(QString imageType);

signals:
    void showImage(unsigned char *image);
};

#endif // IMAGEVIEWER_H
