#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGui>

#define NDEBUG

//#define M_PI       3.14159265358979323846

extern unsigned int imageWidth;
extern unsigned int imageHeight;
inline unsigned int imageSize(){ return imageWidth*imageHeight; }

extern QString workDirectory;

#endif // GLOBALS_H
