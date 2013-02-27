#ifndef IMAGEIO_H
#define IMAGEIO_H

#include "point.h"
#include <string>

struct IMAGE3D
{
    Point coor; //coordinates
    Point norm; //normal vector
    Point text; //texture, intensity
    bool  mask; //mask. true:valid, false: invalid
};

bool writeBytesToFile(const std::string fileName,const unsigned char *buffer,const int size);
bool readBytesFromFile(const std::string fileName,unsigned char *buffer,const int size);

bool writeSingleImage3D(const std::string filename,IMAGE3D *image,const int &width,const int &height);
bool writeMultiImage3D(const std::string filename,IMAGE3D *image,const int &width,const int &height,int num);
bool readSingleImage3D(const std::string fileName,IMAGE3D *image);
bool readMultiImage3D(const std::string fileName,IMAGE3D *image,int num);

bool writeImage3DCoordinates(const std::string fileName,IMAGE3D *image,int width, int height);

//bool writeToPCD(const std::string fileName,IMAGE3D *image,int width,int height);
//bool readFromPCD(IMAGE3D *image, int width, int height, const std::string fileNameCoorNorm);

#endif // IMAGEIO_H
