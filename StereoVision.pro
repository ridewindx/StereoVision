#-------------------------------------------------
#
# Project created by QtCreator 2012-09-09T16:35:21
#
#-------------------------------------------------

QT       += core gui
#QT += widgets

CONFIG += console

TARGET = StereoVision
TEMPLATE = app

QT += opengl

//INCLUDEPATH += $$quote(D:\MATRIX VISION\mvIMPACT acquire)
//LIBS += $$quote(D:\MATRIX VISION\mvIMPACT acquire\lib\mvDeviceManager.lib)

exists( "I:/Program Files/Point Grey Research/FlyCapture2/lib/FlyCapture2.lib" ) {
    DEFINES += HAS_POINT_GREY
    INCLUDEPATH += "I:\Program Files\Point Grey Research\FlyCapture2\include"
    LIBS += "I:\Program Files\Point Grey Research\FlyCapture2\lib\FlyCapture2.lib"
}


win32:INCLUDEPATH += "D:\PCL 1.6.0\include\pcl-1.6" \
                     "D:\PCL 1.6.0\3rdParty\Eigen\include" \
                     "D:\PCL 1.6.0\3rdParty\VTK\include\vtk-5.8" \
                     "D:\PCL 1.6.0\3rdParty\FLANN\include" \
                     "D:\OpenCV\build\include"
win32-msvc2010:INCLUDEPATH += "D:\PCL 1.6.0\3rdParty\Boost\include"
win32-msvc2012:INCLUDEPATH += D:\Boost\include

win32:LIBS_PCL = "-LD:\PCL 1.6.0\lib"
LIBS_PCL_ITEMS = pcl_common pcl_io pcl_kdtree pcl_features pcl_filters \
             pcl_surface pcl_registration pcl_search pcl_sample_consensus \
             pcl_keypoints pcl_visualization
win32:if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
    for(item, LIBS_PCL_ITEMS):LIBS_PCL += -l$${item}_debug
}
win32:if(!debug_and_release|build_pass):CONFIG(release, debug|release) {
    for(item, LIBS_PCL_ITEMS):LIBS_PCL += -l$${item}_release
}
unix:for(item, LIBS_PCL_ITEMS):LIBS_PCL += -l$${item}

win32:LIBS_VTK = "-LD:\PCL 1.6.0\3rdParty\VTK\lib\vtk-5.8"
unix:LIBS_VTK = -L/usr/lib/vtk-5.10
LIBS_VTK_ITEMS = vtkCommon vtkWidgets vtkHybrid vtkCharts \
                 vtkFiltering vtkRendering vtksys vtkGraphics \
                 vtkIO vtkftgl
win32:if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
    for(item, LIBS_VTK_ITEMS):LIBS_VTK += -l$${item}-gd
}
win32:if(!debug_and_release|build_pass):CONFIG(release, debug|release) {
    for(item, LIBS_VTK_ITEMS):LIBS_VTK += -l$${item}
}
unix:for(item, LIBS_VTK_ITEMS):LIBS_VTK += -l$${item}

win32-msvc2010:LIBS_BOOST = "-LD:\PCL 1.6.0\3rdParty\Boost\lib"
win32-msvc2012:LIBS_BOOST = "-LD:\Boost\lib"
win32-msvc2010:if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
    LIBS_BOOST += -llibboost_thread-vc100-mt-gd-1_49
}
win32-msvc2012:if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
    LIBS_BOOST += -llibboost_thread-vc110-mt-gd-1_53
}
win32-msvc2010:if(!debug_and_release|build_pass):CONFIG(release, debug|release) {
    LIBS_BOOST += -llibboost_thread-vc100-mt-1_49
}
win32-msvc2012:if(!debug_and_release|build_pass):CONFIG(release, debug|release) {
    LIBS_BOOST += -llibboost_thread-vc110-mt-1_53
}
unix:LIBS_BOOST += -lboost_thread -lboost_system

win32-msvc2010:LIBS_OPENCV = "-LD:\OpenCV\build\x86\vc10\lib"
win32-msvc2012:LIBS_OPENCV = "-LD:\OpenCV\build\x64\vc11\lib"
win32:if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
    LIBS_OPENCV += -lopencv_core246d \
                   -lopencv_imgproc246d \
                   -lopencv_highgui246d \
                   -lopencv_calib3d246d \
                   -lopencv_features2d246d
}
win32:if(!debug_and_release|build_pass):CONFIG(release, debug|release) {
    LIBS_OPENCV += -lopencv_core246 \
                   -lopencv_imgproc246 \
                   -lopencv_highgui246 \
                   -lopencv_calib3d246 \
                   -lopencv_features2d246
}


LIBS += $$LIBS_PCL $$LIBS_VTK $$LIBS_BOOST $$LIBS_OPENCV



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
    #cameramv.cpp \
    cameraviewer.cpp \
    calib.cpp \
    features.cpp \
    camerapointgrey.cpp

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
    #cameramv.h \
    cameraviewer.h \
    calib.h \
    features.h \
    camerapointgrey.h

FORMS    += mainwindow.ui \
    imageviewer.ui \
    projection.ui \
    imageshow.ui \
    cameraviewer.ui

#LIBS += D:/PCL/lib/ -lpcl_common_debug
#LIBS += D:/PCL/lib/ -lpcl_io_debug
#LIBS += D:/PCL/lib/ -lpcl_features_debug
#LIBS += D:/PCL/lib/ -lpcl_filters_debug
#LIBS += D:/PCL/lib/ -lpcl_kdtree_debug
#LIBS += D:/PCL/lib/ -lpcl_keypoints_debug
#LIBS += D:/PCL/lib/ -lpcl_ml_debug
#LIBS += D:/PCL/lib/ -lpcl_octree_debug
#LIBS += D:/PCL/lib/ -lpcl_recognition_debug
#LIBS += D:/PCL/lib/ -lpcl_registration_debug
#LIBS += D:/PCL/lib/ -lpcl_sample_consensus_debug
#LIBS += D:/PCL/lib/ -lpcl_search_debug
#LIBS += D:/PCL/lib/ -lpcl_segmentation_debug
#LIBS += D:/PCL/lib/ -lpcl_stereo_debug
#LIBS += D:/PCL/lib/ -lpcl_surface_debug
#LIBS += D:/PCL/lib/ -lpcl_tracking_debug
#LIBS += D:/PCL/lib/ -lpcl_visualization_debug


#INCLUDEPATH += $$PWD/D:/PCL/include/pcl-1.7
#DEPENDPATH += $$PWD/D:/PCL/include/pcl-1.7
