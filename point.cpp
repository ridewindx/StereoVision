#include "point.h"

double& Point:: operator[]( int i)
{
    assert( i>=0 && i<3 );
    return value[i];
}

double  Point:: operator[]( int i) const
{
    assert( i>=0 && i<3 );
    return value[i];
}

double  Point:: operator()( int i) const
{
    assert( i>=0 && i<3 );
    return value[i];
}

//L2范数
double Point::norm() const
{
    return sqrt(fabs(value[0]*value[0]+value[1]*value[1]+value[2]*value[2]));
}

Point& Point::operator+=(const Point &p)
{
    for ( int i = 0; i < 3; i++)
        value[i]+=p(i);
    return *this;
}

Point& Point::operator-=(const Point &p)
{
    for ( int i = 0; i < 3; i++)
        value[i]-=p(i);
    return *this;
}

Point& Point:: operator*=(const double scale)
{
    for ( int i = 0; i < 3; i++)
        value[i]*=scale;
    return *this;
}

Point& Point:: operator/=(const double scale)
{
    for ( int i = 0; i < 3; i++)
        value[i]/=scale;
    return *this;
}

double Point:: operator*(const Point &p) const
{
    return value[0]*p[0]+value[1]*p[1]+value[2]*p[2];
}

Point Point::operator+(const Point &p) const
{
    Point  r(value[0]+p[0],value[1]+p[1],value[2]+p[2]);
    return r;
}

Point Point::operator-(const Point &p) const
{
    Point r(value[0]-p[0],value[1]-p[1],value[2]-p[2]);
    return r;
}

Point Point::operator*(const double scale) const
{
    Point r(value[0]*scale,value[1]*scale,value[2]*scale );
    return r;
}

Point Point:: operator/(const double scale) const
{
    Point r(value[0]/scale,value[1]/scale,value[2]/scale);
    return r;
}

Point Point::operator-() const
{
    Point p(-value[0],-value[1],-value[2]);
    return p;
}

//Cross-product向量积
Point Point::operator^(const Point &p) const
{
    Point r(value[1]*p[2]-value[2]*p[1],
            value[2]*p[0]-value[0]*p[2],
            value[0]*p[1]-value[1]*p[0]
            );
    return r;
}

std::ostream& operator<<(std::ostream & os, const Point &q)
{
    os<<"Quaternion: ( "<<q(0)<<", "<<q(1)<<", "<<q(2)<<" )"<<std::endl;
    return os;
}
