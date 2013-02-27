#ifndef MODELSHOW_H
#define MODELSHOW_H

#include <QtGUI>

#include <QtOpenGL>

#include "imageIO.h"

class ModelShow : public QGLWidget
{
public:
    ModelShow(int width,int height,QWidget *parent=0, QGLWidget *shareWidget=0);
    void setImage(IMAGE3D *image);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void translateBy(double x,double y,double z);
    void rotateBy(int xAngle,int yAngle,int zAngle);
    void zoomBy(double z);

    void scale();
    void make();

    void drawModel();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width,int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *);

private:
    int width; //图像尺寸
    int height;
    int size;
    IMAGE3D *image;

    QPoint lastPos;
    double xTrans;
    double yTrans;
    double zTrans;
    int xRot;
    int yRot;
    int zRot;
    double zoom;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;

    double aspectRatio;

    int renderMode;
    int renderColor;
};

#endif // MODELSHOW_H
