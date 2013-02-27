#include "modelshow.h"


ModelShow::ModelShow(int width,int height, QWidget *parent, QGLWidget *shareWidget):QGLWidget(parent,shareWidget)
{
    setFocusPolicy(Qt::StrongFocus);

    this->width=width;
    this->height=height;
    size=width*height;

    xTrans=0.0f;
    yTrans=0.0f;
    zTrans=0.0f;
    xRot=0;
    yRot=0;
    zRot=0;
    zoom=1.0f;

    renderMode=GL_FILL;
    renderColor=2;
}

void ModelShow::setImage(IMAGE3D *image)
{
    this->image=image;
}

QSize ModelShow::minimumSizeHint() const
{
    return QSize(100,100);
}

QSize ModelShow::sizeHint() const
{
    return QSize(400,400);
}

void ModelShow::translateBy(double x, double y, double z)
{
    xTrans+=x;
    yTrans+=y;
    zTrans+=z;
    updateGL();
}

void ModelShow::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot+=xAngle;
    yRot+=yAngle;
    zRot+=zAngle;
    updateGL();
}

void ModelShow::zoomBy(double z)
{
    zoom+=z;
    if (zoom<=0.1)
        zoom=0.1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0*zoom*aspectRatio,+1.0*zoom*aspectRatio,-1.0*zoom,1.0*zoom,-2.0,2.0);
    glMatrixMode(GL_MODELVIEW);
    updateGL();
}

void ModelShow::scale()
{
    static Point ptMax(-1E9,-1E9,-1E9);
    static Point ptMin(+1E9,+1E9,+1E9);

    for(int i=0;i<size;i++){
        if(image[i].mask){
            Point pt=image[i].coor;
            for(int k=0;k<3;k++){
                ptMax[k]=ptMax[k]>pt[k]?ptMax[k]:pt[k];
                ptMin[k]=ptMin[k]<pt[k]?ptMin[k]:pt[k];
            }
        }
    }
    Point c=(ptMax+ptMin)*0.5; //包络长方体的中心坐标
    Point d=ptMax-ptMin; //包络长方体的长宽高
    //double s=1/(d[0]>d[1]?d[0]:d[1])*2;
    double s=1/(d[0]>d[1]?d[0]:d[1])*1;

    for(int i=0;i<size;i++){
        if(image[i].mask){
            image[i].coor -= c; //中心移到原点
            image[i].coor *= s; //坐标缩放到最多1范围内
        }
    }
}

void ModelShow::make()
{
    vertices.clear();
    normals.clear();
    Point c[4],n[4],t[4];
    bool m[4];
    int idx[4][2]={{0,0},{1,0},{0,1},{1,1}};
    for(int i=1;i<height-1;i++){
        for(int j=1;j<width-1;j++){
            for(int k=0;k<4;k++){
                int id=(i+idx[k][0])*width+(j+idx[k][1]);
                c[k]=image[id].coor;
                n[k]=image[id].norm;
                t[k]=image[id].text;
                m[k]=image[id].mask;
            }
            if(m[0]&&m[1]&&m[2]){
                /*
                vertices.append(QVector3D(c[0][0],c[0][1],c[0][2]));
                normals.append(QVector3D(n[0][0],n[0][1],n[0][2]));
                vertices.append(QVector3D(c[1][0],c[1][1],c[1][2]));
                normals.append(QVector3D(n[1][0],n[1][1],n[1][2]));
                vertices.append(QVector3D(c[2][0],c[2][1],c[2][2]));
                normals.append(QVector3D(n[2][0],n[2][1],n[2][2]));
                */
                /*重构的世界坐标系--->OpenGL坐标系*/
                vertices.append(QVector3D(c[0][1],-c[0][0],c[0][2]));
                normals.append(QVector3D(n[0][1],-n[0][0],n[0][2]));
                vertices.append(QVector3D(c[1][1],-c[1][0],c[1][2]));
                normals.append(QVector3D(n[1][1],-n[1][0],n[1][2]));
                vertices.append(QVector3D(c[2][1],-c[2][0],c[2][2]));
                normals.append(QVector3D(n[2][1],-n[2][0],n[2][2]));
            }
            if(m[1]&&m[3]&&m[2]){
                /*
                vertices.append(QVector3D(c[1][0],c[1][1],c[1][2]));
                normals.append(QVector3D(n[1][0],n[1][1],n[1][2]));
                vertices.append(QVector3D(c[3][0],c[3][1],c[3][2]));
                normals.append(QVector3D(n[3][0],n[3][1],n[3][2]));
                vertices.append(QVector3D(c[2][0],c[2][1],c[2][2]));
                normals.append(QVector3D(n[2][0],n[2][1],n[2][2]));
                */
                vertices.append(QVector3D(c[1][1],-c[1][0],c[1][2]));
                normals.append(QVector3D(n[1][1],-n[1][0],n[1][2]));
                vertices.append(QVector3D(c[3][1],-c[3][0],c[3][2]));
                normals.append(QVector3D(n[3][1],-n[3][0],n[3][2]));
                vertices.append(QVector3D(c[2][1],-c[2][0],c[2][2]));
                normals.append(QVector3D(n[2][1],-n[2][0],n[2][2]));
            }
        }
    }
}

void ModelShow::drawModel()
{
    scale();
    make();
    updateGL();
}

void ModelShow::initializeGL()
{
    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    glFrontFace(GL_CCW);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_COLOR_MATERIAL);

    glPolygonMode(GL_FRONT_AND_BACK,renderMode);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    static GLfloat lightPosition[4]={0.5,5.0,7.0,1.0 };
    glLightfv(GL_LIGHT0,GL_POSITION,lightPosition);
}

void ModelShow::paintGL()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(renderColor==0)
        glColor3f(1.165f,0.775f,0.645f);
    else if(renderColor==1)
        glColor3f(0.0f,0.0f,1.0f);
    else
        glColor3f(0.106f,0.749f,1.0f);

    glLoadIdentity();
    glTranslatef(xTrans,yTrans,zTrans);
    glRotatef(xRot/16.0f,1.0f,0.0f,0.0f);
    glRotatef(yRot/16.0f,0.0f,1.0f,0.0f);
    glRotatef(zRot/16.0f,0.0f,0.0f,1.0f);

    glVertexPointer(3,GL_FLOAT,0,vertices.constData());
    glEnableClientState(GL_VERTEX_ARRAY);

    glNormalPointer(GL_FLOAT,0,normals.constData());
    glEnableClientState(GL_NORMAL_ARRAY);

    glDrawArrays(GL_TRIANGLES,0,vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void ModelShow::resizeGL(int width, int height)
{
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    aspectRatio=double(width)/double(height);
    glOrtho(-1.0*zoom*aspectRatio,+1.0*zoom*aspectRatio,-1.0*zoom,1.0*zoom,-2.0,2.0);
    glMatrixMode(GL_MODELVIEW);
}

void ModelShow::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void ModelShow::mouseMoveEvent(QMouseEvent *event)
{
    int dx=event->x()-lastPos.x();
    int dy=event->y()-lastPos.y();

    if(event->buttons() & Qt::LeftButton){
        rotateBy(-8*dy,8*dx,0);
    }
    else if(event->buttons() & Qt::RightButton){
        zoomBy(-0.01*dx);
    }
    else if(event->buttons() & Qt::MiddleButton){
        translateBy(0.01*dx,0.01*dy,0);
    }
    lastPos=event->pos();
}

void ModelShow::mouseReleaseEvent(QMouseEvent *event)
{
}

void ModelShow::keyPressEvent(QKeyEvent *)
{
    //renderColor=(renderColor+1)%3;
    updateGL();
}
