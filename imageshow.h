#ifndef IMAGESHOW_H
#define IMAGESHOW_H

#include <QDialog>

namespace Ui {
class ImageShow;
}

class ImageShow : public QDialog
{
    Q_OBJECT
    
public:
    explicit ImageShow(QWidget *parent = 0);
    ~ImageShow();

    void init(QString imageName,int width,int height,unsigned char *image=NULL)
    {
        this->imageName=imageName;
        this->width=width;this->height=height;
        size=width*height;
        if(imageName==tr("directshow") && image!=NULL){
            this->image=image;
        }
    }

    void paintEvent(QPaintEvent *);
    
private:
    Ui::ImageShow *ui;

    int width,height;
    int size;

    QString imageName;

    unsigned char *image;
};

#endif // IMAGESHOW_H
