#include "globals.h"

#define IMG1024
//#define IMG640
//#define IMG1600
//#define IMG2448

#ifdef IMG1024
unsigned int imageWidth=1024;
unsigned int imageHeight=768;
#endif
#ifdef IMG640
unsigned int imageWidth=640;
unsigned int imageHeight=480;
#endif
#ifdef IMG1600
unsigned int imageWidth=1600;
unsigned int imageHeight=1200;
#endif
#ifdef IMG2448
unsigned int imageWidth=2448;
unsigned int imageHeight=2048;
#endif

QString workDirectory = "";
