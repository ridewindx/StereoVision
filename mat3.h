#ifndef MAT3_H
#define MAT3_H

#include "point.h"
#include <iostream>
#include <cmath>

class Mat3
{
    friend std::ostream &operator<<(std::ostream &out,const Mat3 &m);
public:
    Mat3()
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                m[i][j]=0;
            }
        }
    }
    Mat3(const double m[3][3])
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                this->m[i][j]=m[i][j];
            }
        }
    }

    Mat3(const Mat3 &mat3)
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                m[i][j]=mat3.m[i][j];
            }
        }
    }

    static Mat3 zeros(){ Mat3 mat3; return mat3;}
    static Mat3 ones()
    {
        Mat3 mat3;
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                mat3.m[i][j]=1;
            }
        }
        return mat3;
    }

    static Mat3 eye()
    {
        Mat3 mat3;
        for(int i=0;i<3;i++){ mat3.m[i][i]=1;}
        return mat3;
    }

    Mat3 &operator=(const Mat3 &mat3)
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                m[i][j]=mat3.m[i][j];
            }
        }
        return *this;
    }

    Mat3 &operator+=(const Mat3 &mat3)
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                m[i][j]+=mat3.m[i][j];
            }
        }
        return *this;
    }
    Mat3 &operator*=(const Mat3 &mat3)
    {
        Mat3 newmat3;
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                newmat3.m[i][j]=m[i][0]*mat3.m[0][j]+m[i][1]*mat3.m[1][j]+m[i][2]*mat3.m[2][j];
            }
        }
        *this=newmat3;
        return *this;
    }

    Mat3 &operator*=(const double &scale)
    {
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                m[i][j]*=scale;
            }
        }
        return *this;
    }

    Mat3 inverse()
    {
        Mat3 mat3;
        double invmat3[3][3];
        invmat3[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
        invmat3[0][1] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
        invmat3[0][2] = m[1][0]*m[2][1] - m[2][0]*m[1][1];
        invmat3[1][0] = m[0][2]*m[2][1] - m[0][1]*m[2][2];
        invmat3[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
        invmat3[1][2] = m[2][0]*m[0][1] - m[0][0]*m[2][1];
        invmat3[2][0] = m[0][1]*m[1][2] - m[1][1]*m[0][2];
        invmat3[2][1] = m[1][0]*m[0][2] - m[0][0]*m[1][2];
        invmat3[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];

        double det = m[0][0]*invmat3[0][0] + m[0][1]*invmat3[0][1] + m[0][2]*invmat3[0][2];

        if (fabs(det) < 1E-9)
        {
                std::cout << "Matrix is noninversible" << std::endl;
                det = 1E-9f;
        }

        double inv = 1.0 / det;

        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 3; col++)
                mat3.m[row][col] = invmat3[col][row]*inv;
        return mat3;
    }

    double m[3][3];
};

inline Mat3 operator+(const Mat3 &mat31,const Mat3 &mat32)
{
    Mat3 newmat3;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            newmat3.m[i][j]=mat31.m[i][j]+mat32.m[i][j];
        }
    }
    return newmat3;
}

inline Mat3 operator*(const Mat3 &mat31,const Mat3 &mat32)
{
    Mat3 newmat3;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            newmat3.m[i][j]=mat31.m[i][0]*mat32.m[0][j]+mat31.m[i][1]*mat32.m[1][j]+mat31.m[i][2]*mat32.m[2][j];
        }
    }
    return newmat3;
}

inline Mat3 operator*(const Mat3 &mat3,const double &scale)
{
    Mat3 newmat3;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            newmat3.m[i][j]=mat3.m[i][j]*scale;
        }
    }
    return newmat3;
}

inline Point operator*(const Mat3 &mat3,const Point &p)
{
    Point newp;
    for(int i=0;i<3;i++){
        newp[i]=mat3.m[i][0]*p[0]+mat3.m[i][1]*p[1]+mat3.m[i][2]*p[2];
    }
    return newp;
}

#endif // MAT3_H
