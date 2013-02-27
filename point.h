#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <cassert>
#include <cmath>

class Point
{
public:
    Point(double x=0,double y=0,double z=0){
        this->x=x;this->y=y;this->z=z;
    }
    Point(double value[3]){
        this->x=value[0];this->y=value[1];this->z=value[2];
    }

    double& operator[](int i);
    double  operator[](int i) const;
    double  operator()(int i) const;

    double  norm() const;

    Point& operator += (const Point &p);
    Point& operator -= (const Point &p);
    Point& operator *= (const double scale);
    Point& operator /= (const double scale);

    double   operator*(const Point &p) const;

    Point operator+(const Point &p) const;
    Point operator-(const Point &p) const;
    Point operator*(const double s) const;
    Point operator/(const double s) const;
    Point operator^(const Point &p2) const;
    Point operator-() const;

    friend std::ostream& operator<<(std::ostream &os,const Point &p);

    //data members
    union{
        struct{
            double x;
            double y;
            double z;
        };
        double value[3];
    };
};

std::ostream& operator<<(std::ostream &os, const Point &p);

#endif // POINT_H
