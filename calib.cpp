#include "calib.h"

#include <QTextStream>
#include <QDataStream>
#include <iostream>
#include <cmath>

#include <opencv2/calib3d/calib3d.hpp>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include "globals.h"
#include "twocamreconstrutor.h"

namespace calib {

// left camera
double fc_l[2]; // focal length
double cc_l[2]; // principal point
double alpha_l; // skew coefficient
double kc_l[5]; // distortion coefficients

double t_l[3];
double r_l[9];

// right camera
double fc_r[2];
double cc_r[2];
double alpha_r;
double kc_r[5];

double t_r[3];
double r_r[9];

bool readSingleCalibParams(const QString &filename, double *fc, double *cc,
                                  double &alpha, double *kc, double *t, double *r)
{
    bool success = true;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Can't open file \"" << filename.toStdString() << "\"." << endl;
        return false;
    }
    QTextStream ts(&file);
    QString word;
    while (!ts.atEnd()) {
        ts >> word;
        if (word == "fc") {
            ts >> word;
            if (word == "=") {
                ts >> word;
                if (word == "[") {
                    ts >> fc[0];
                    ts >> word;
                    if (word != ";") {
                        success = false;
                        break;
                    }
                    ts >> fc[1];
                }
            }
        }
        if (word == "cc") {
            ts >> word;
            if (word == "=") {
                ts >> word;
                if (word == "[") {
                    ts >> cc[0];
                    ts >> word;
                    if (word != ";") {
                        success = false;
                        break;
                    }
                    ts >> cc[1];
                }
            }
        }
        if (word == "alpha_c") {
            ts >> word;
            if (word == "=") {
                ts >> alpha;
            }
        }
        if (word == "kc") {
            ts >> word;
            if (word == "=") {
                ts >> word;
                if (word == "[") {
                    ts >> kc[0];
                    for (int i = 1; i < 5; i++) {
                        ts >> word;
                        if (word != ";") {
                            success = false;
                            break;
                        }
                        ts >> kc[i];
                    }
                    if (success == false)
                        break;
                }
            }
        }
        cv::Mat rot(1, 3, CV_64F);;
        if (word == "omc_1") {
            ts >> word;
            if (word == "=") {
                ts >> word;
                if (word == "[") {
                    ts >> rot.at<double>(0,0);
                    for (int i = 1; i < 3; i++) {
                        ts >> word;
                        if (word != ";") {
                            success = false;
                            break;
                        }
                        ts >> rot.at<double>(0,i);
                    }
                    if (success == false)
                        break;
                    cv::Mat rot_r(3, 3, CV_64F);
                    cv::Rodrigues(rot, rot_r);
                    for (int i = 0; i < 9; i++) {
                        r[i] = rot_r.at<double>(i/3,i%3);
                    }
                }
            }
        }
        if (word == "Tc_1") {
            ts >> word;
            if (word == "=") {
                ts >> word;
                if (word == "[") {
                    ts >> t[0];
                }
                for (int i = 1; i < 3; i++) {
                    ts >> word;
                    if (word != ";") {
                        success = false;
                        break;
                    }
                    ts >> t[i];
                }
                if (success == false)
                    break;
            }
        }
    }
    return success;
}

bool readCalibParams(const QString filenames[2])
{
    bool l = readSingleCalibParams(filenames[0], fc_l, cc_l, alpha_l, kc_l, t_l, r_l);
    bool r = readSingleCalibParams(filenames[1], fc_r, cc_r, alpha_r, kc_r, t_r, r_r);
    std::cout<<fc_l[0]<<", "<<fc_l[1]<<", "<<cc_l[0]<<", "<<cc_l[1]<<", "<<alpha_l<<"\n";
    std::cout<<kc_l[0]<<", "<<kc_l[1]<<", "<<kc_l[2]<<", "<<kc_l[3]<<", "<<kc_l[4]<<", "<<"\n";
    std::cout<<t_l[0]<<", "<<t_l[1]<<", "<<t_l[2]<<", "<<"\n";
    std::cout<<r_l[0]<<", "<<r_l[1]<<", "<<r_l[2]<<", "<<"\n";
    std::cout<<r_l[3]<<", "<<r_l[4]<<", "<<r_l[5]<<", "<<"\n";
    std::cout<<r_l[6]<<", "<<r_l[7]<<", "<<r_l[8]<<", "<<"\n";
    std::cout<<std::flush;
    return l && r;
}

inline void normalize(float x[2] /*input and output*/, double *fc, double *cc,
               double &alpha, double *kc)
{
    x[0] = (x[0] - cc[0]) / fc[0];
    x[1] = (x[1] - cc[1]) / fc[1];
    x[0] = x[0] - alpha * x[1];

    double k1 = kc[0];
    double k2 = kc[1];
    double k3 = kc[4];
    double p1 = kc[2];
    double p2 = kc[3];
    float xd0=x[0], xd1=x[1]; // distorted value
    for (int i = 0; i < 20; i++) { // iterate to solve nonlinear equations
        double r_2 = std::pow(x[0], 2) + std::pow(x[1], 2);
        double k_radial = 1 + k1 * r_2 +
                k2 * std::pow(r_2, 2) + k3 * std::pow(r_2, 3);
        double delta_x[2];
        delta_x[0] = 2 * p1 * x[0] * x[1] + p2 * (r_2 + 2*std::pow(x[0], 2));
        delta_x[1] = p1 * (r_2 + 2*std::pow(x[1],2) + 2 * p2 * x[0] * x[1]);
        x[0] = (xd0 - delta_x[0]) / k_radial;
        x[1] = (xd1 - delta_x[1]) / k_radial;
    }
}

bool computeNormalizedProjections(const QString filenames[2])
{
    int width = imageWidth;
    int height = imageHeight;

    float *xs = new float[width * height * 2];

    QFile file(filenames[0]);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        std::cerr << "Can't open file \"" << filenames[0].toStdString() << "\"." << endl;
        delete [] xs;
        return false;
    }
    QDataStream ds(&file);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (int i = 0; i < width; i++) { // from top to bottom, from left to right
        for (int j = 0; j < height; j++) {
            xs[2 * (i*height + j)] = i;
            xs[2 * (i*height + j) + 1] = j;
            normalize(&xs[2 * (i*height + j)],
                    fc_l, cc_l, alpha_l, kc_l);
            ds.writeRawData((char *)&xs[2 * (i*height +j)], 8);
        }
    }
    file.close();

    file.setFileName(filenames[1]);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        std::cerr << "Can't open file \"" << filenames[1].toStdString() << "\"." << endl;
        delete [] xs;
        return false;
    }
    ds.setDevice(&file);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            xs[2 * (i*height + j)] = i;
            xs[2 * (i*height + j) + 1] = j;
            normalize(&xs[2 * (i*height + j)],
                    fc_r, cc_r, alpha_r, kc_r);
            ds.writeRawData((char *)&xs[2 * (i*height +j)], 8);
        }
    }
    file.close();

    delete [] xs;
    return true;
}

void calibrateRotaryTable(const QVector<QVector<Vector3f> > &points,
                          Vector3f &center, Vector3f &direction)
{
    center << 0.0f, 0.0f, 0.0f;
    direction << 0.0f, 0.0f, 0.0f;
    //int nc = 0, nd = 0;
    for (int i = 0; i < points.size(); i++) {
        const QVector<Vector3f> &ps = points[i];

        MatrixXf A;
        A.resize(ps.size(), 4);
        MatrixXf b;
        b.resize(ps.size(), 1);
        Vector4f coes;

        int /*inc=0,*/ind=0;
        Vector3f icenter(0.0f,0.0f,0.0f), idirection(0.0f,0.0f,0.0f);
        for (int j = 0; j < ps.size(); j++) {
            A.row(j) << ps[j](0),ps[j](1),ps[j](2),-1;
            b.row(j) << ps[j].squaredNorm();

            //center += ps[j];
            //nc++;

            //icenter += ps[j];
            //inc++;
        }
        coes = (A.transpose() * A).inverse() * A.transpose() * b;
        icenter << coes(0)/2,coes(1)/2,coes(2)/2;
        center += icenter;

        for (int j = 0; j < ps.size()-1; j++) {
            for (int k = j+1; k < ps.size(); k++) {
                Vector3f v1 = ps[j] - icenter;
                Vector3f v2 = ps[k] - icenter;
                //direction += Vector3f(v1.cross(v2)).normalized();
                //nd++;

                idirection += Vector3f(v1.cross(v2)).normalized();
                ind++;
            }
        }

        //icenter /= inc;
        idirection /= ind;
        idirection.normalize();
        direction += idirection;
        //std::cout<<"Iter "<<i<<": center ("<<icenter(0)<<","<<icenter(1)<<","<<icenter(2)<<")";
        //std::cout << ", direction ("<<idirection(0)<<","<<idirection(1)<<","<<idirection(2)<<")\n";
        std::cout<<"c"<<i<<"=["<<icenter(0)<<","<<icenter(1)<<","<<icenter(2)<<"]";
        std::cout << "; d"<<i<<"=["<<idirection(0)<<","<<idirection(1)<<","<<idirection(2)<<"];\n";
    }
    center /= points.size();
    direction /= points.size();
    direction.normalize();
    std::cout<<"c=["<<center(0)<<","<<center(1)<<","<<center(2)<<"]";
    std::cout << "; d=["<<direction(0)<<","<<direction(1)<<","<<direction(2)<<"];\n";
}

void transformPoint(Vector3f &point, float angle /*rotation angle*/,
                    const Vector3f &center, const Vector3f &direction)
{
    AngleAxisf aa(angle, direction);
    point = point - center;
    point = aa * point;
    point = point + center;
}

void test()
{
    int width = imageWidth;
    int height = imageHeight;
    TwocamReconstrutor *rePtr=new TwocamReconstrutor(width, height);
    if (!rePtr->init()) {
        return;
    }
    QVector<Vector2i> ps_l;
    QVector<Vector2i> ps_r;
    QVector<Vector3f> ps;

    int L[][2] = /*{620,337, 657,824, 742,1505, 756,312, 794,818, 870,1525,
                  1113,282, 1143,813, 1187,1544, 1434,276, 1453,809, 1463,1545,
                  1788,287, 1790,811, 1761,1531, 2080,323, 2066,816, 2004,1500};*/
                 {410,283, 421,439, 438,656, 511,283, 520,442, 532,661,
                  602,283, 608,444, 614,663, 662,285, 665,444, 668,661,
                  710,288, 710,444, 710,661, 770,289, 767,444, 761,657,
                  /*799,289, 794,442, 785,654, 838,292, 830,442, 817,648*/};
    int R[][2] = /*{497,406, 528,879, 609,1547, 569,379, 603,874, 683,1567,
                  838,342, 871,869, 933,1599, 1134,326, 1159,866, 1192,1612,
                  1515,327, 1525,866, 1516,1608, 1897,351, 1888,869, 1838,1587};*/
                 {265,256, 277,411, 298,628, 347,253, 359,414, 379,634,
                  431,252, 442,414, 458,637, 493,252, 502,414, 514,637,
                  545,252, 551,414, 560,636, 617,253, 620,412, 623,633,
                  /*653,255, 655,412, 655,631, 709,256, 707,411, 703,625*/};

    int num = sizeof(L)/sizeof(int) / 2;
    for (int i = 0; i < num; i++) {
        ps_l.append(Vector2i(L[i][0], L[i][1]));
        ps_r.append(Vector2i(R[i][0], R[i][1]));
    }


    for (int i = 0; i < ps_l.size(); i++) {
        int x_l = ps_l[i](0);
        int y_l = ps_l[i](1);
        int x_r = ps_r[i](0);
        int y_r = ps_r[i](1);
        double u_l = rePtr->xncamL[(x_l*height+y_l)*2];
        double v_l = rePtr->xncamL[(x_l*height+y_l)*2+1];
        double u_r = rePtr->xncamR[(x_r*height+y_r)*2];
        double v_r = rePtr->xncamR[(x_r*height+y_r)*2+1];
        Point l(u_l, v_l, 1);
        Point r(u_r, v_r, 1);
        Point p = rePtr->computeCoor(r, l, 1);
        ps.append(Vector3f(p.x, p.y, p.z));
        //std::cout << "Pixels: (" << x_l<<","<<y_l<<"), ("<<x_r<<","<<y_r<<") -> Point ("
        //          << p.x<<","<<p.y<<","<<p.z<<")\n";
        //std::cout<<p.x<<","<<p.y<<","<<p.z<<";\n";
    }



    pcl::PointCloud<pcl::PointXYZ> cloud;
    cloud.reserve(30);
    QVector<QVector<Vector3f> > points;
    points.resize(3);
    for (int i = 0; i < 3; i++) {
        QVector<Vector3f> &pv = points[i];
        for (int j = 0; j < num/3; j++) {
            Vector3f &pt = ps[j*3 + i];
            pv.append(pt);
            std::cout<<pt(0)<<","<<pt(1)<<","<<pt(2)<<";\n";
            cloud.push_back(pcl::PointXYZ(pt(0),pt(1),pt(2)));
        }
    }
    Vector3d avcenter(54.3942,   70.7414,   43.9948);
    Vector3d avdirection(-0.9471,   -0.0244,    0.3200);
    cloud.push_back(pcl::PointXYZ(avcenter(0),avcenter(1),avcenter(2)));
    for (int i = -10; i < 10; i++) {
        cloud.push_back(pcl::PointXYZ(avcenter(0)+i*avdirection(0),avcenter(1)+i*avdirection(1),avcenter(2)+i*avdirection(2)));
    }

    /* test for one point */
    /*
    double u_l = rePtr->xncamL[(1483*height+1668)*2];
    double v_l = rePtr->xncamL[(1483*height+1668)*2+1];
    double u_r = rePtr->xncamR[(1442*height+1733)*2];
    double v_r = rePtr->xncamR[(1442*height+1733)*2+1];
    Point l(u_l, v_l, 1);
    Point r(u_r, v_r, 1);
    Point p = rePtr->computeCoor(r, l, 1);
    cloud.push_back(pcl::PointXYZ(p.x, p.y, p.z));
    */

    pcl::io::savePCDFileBinary((workDirectory+"/points.pcd").toStdString(), cloud);

    Vector3f center;
    Vector3f direction;
    calibrateRotaryTable(points, center, direction);
    std::cout << "Rotary Table: center ("<<center(0)<<","<<center(1)<<","<<center(2)<<")";
    std::cout << ", direction ("<<direction(0)<<","<<direction(1)<<","<<direction(2)<<")\n";

    delete rePtr;
}

void test2()
{
    Vector3d center(54.3942,   70.7414,   43.9948);
    Vector3d direction(-0.9471,   -0.0244,    0.3200);
    double angle = 10 *M_PI/180.0;
    AngleAxisd aa (angle, direction);
    Matrix3d rot = aa.toRotationMatrix();
    std::cout<<"rot: "<< rot <<"\n";
    Matrix4d t1; t1 << (Matrix3d().Identity()),center,0,0,0,1;
    std::cout<<"t1: "<< t1 <<"\n";
    Matrix4d r; r <<rot,Vector3d(0,0,0),0,0,0,1;
    std::cout<<"r: "<< r <<"\n";
    Matrix4d t2; t2 << (Matrix3d().Identity()),-center,0,0,0,1;
    std::cout<<"t2: "<< t2 <<"\n";
    Matrix4d m = t1 * r * t2;

    std::cout<<"m: "<< m <<"\n";

    angle = -10 *M_PI/180.0;
    AngleAxisd aa1 (angle, direction);
    Matrix3d rot1 = aa1.toRotationMatrix();
    std::cout<<"rot1: "<<rot1 <<"\n";
    Matrix4d t11; t11 << (Matrix3d().Identity()),center,0,0,0,1;
    std::cout<<"t11: "<< t11 <<"\n";
    Matrix4d r1; r1 <<rot,Vector3d(0,0,0),0,0,0,1;
    std::cout<<"r1: "<< r1 <<"\n";
    Matrix4d t21; t21<< (Matrix3d().Identity()),-center,0,0,0,1;
    std::cout<<"t21: "<< t21 <<"\n";
    Matrix4d m1 = t11 * r1 * t21;
    std::cout<<"m1: "<< m1 <<"\n";
}
}
