#ifndef CALIB_H
#define CALIB_H

#include <QString>
#include <QVector>
#include <Eigen/Geometry>

using namespace Eigen;

namespace calib {

bool readCalibParams(const QString filenames[2]);
bool computeNormalizedProjections(const QString filenames[2]);

void calibrateRotaryTable(const QVector<QVector<Vector3f> > &points,
                          Vector3f &center, Vector3f &direction);
void transformPoint(Vector3f &point, float angle,
                    const Vector3f &center, const Vector3f &direction);
void reconstructPoints();
void test();
void test2();


// left camera
extern double fc_l[2]; // focal length
extern double cc_l[2]; // principal point
extern double alpha_l; // skew coefficient
extern double kc_l[5]; // distortion coefficients

extern double t_l[3]; // translation vector
extern double r_l[9]; // rotation matrix

// right camera
extern double fc_r[2];
extern double cc_r[2];
extern double alpha_r;
extern double kc_r[5];

extern double t_r[3];
extern double r_r[9];
}

#endif // CALIB_H
