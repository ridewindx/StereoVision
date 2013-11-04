#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGui>

#define NDEBUG

//#define M_PI       3.14159265358979323846

// 相机宽高
extern unsigned int imageWidth;
extern unsigned int imageHeight;
inline unsigned int imageSize(){ return imageWidth*imageHeight; }

extern QString workDirectory; // 全局工作目录

inline bool isInWorkspace() { return !workDirectory.isEmpty(); }

#endif // GLOBALS_H
