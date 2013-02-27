#-------------------------------------------------
#
# Project created by QtCreator 2012-09-09T16:35:21
#
#-------------------------------------------------

QT       += core gui
#QT += widgets

TARGET = StereoVision
TEMPLATE = app

QT += opengl

INCLUDEPATH += $$quote(D:\MATRIX VISION\mvIMPACT acquire)
LIBS += $$quote(D:\MATRIX VISION\mvIMPACT acquire\lib\mvDeviceManager.lib)


INCLUDEPATH += D:\Eigen\include
INCLUDEPATH += D:\Boost\include



SOURCES += main.cpp\
        mainwindow.cpp \
    camera.cpp \
    cameraPulnix.cpp \
    point.cpp \
    imageIO.cpp \
    twocamreconstrutor.cpp \
    imageviewer.cpp \
    globals.cpp \
    projection.cpp \
    combinedmodelmaker.cpp \
    phase.cpp \
    imageshow.cpp \
    modelshow.cpp \
    cameramv.cpp \
    cameraviewer.cpp

HEADERS  += mainwindow.h \
    camera.h \
    cameraPulnix.h \
    point.h \
    imageIO.h \
    mat3.h \
    reconstructor.h \
    twocamreconstrutor.h \
    imageviewer.h \
    globals.h \
    projection.h \
    combinedmodelmaker.h \
    phase.h \
    imageshow.h \
    modelshow.h \
    cameramv.h \
    cameraviewer.h

FORMS    += mainwindow.ui \
    imageviewer.ui \
    projection.ui \
    imageshow.ui \
    cameraviewer.ui

win32: LIBS += -L$$PWD/D:/PCL/lib/ -lpcl_common_debug
win32: LIBS += -L$$PWD/D:/PCL/lib/ -lpcl_io_debug

INCLUDEPATH += $$PWD/D:/PCL/include/pcl-1.7
DEPENDPATH += $$PWD/D:/PCL/include/pcl-1.7
