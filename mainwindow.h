#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "globals.h"
#include "camera.h"
#include "camerapointgrey.h"
#include "cameraviewer.h"
#include "projection.h"
#include "combinedmodelmaker.h"
#include "modelshow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionHorizontal_1_triggered();

    void on_actionHorizontal_2_triggered();

    void on_actionHorizontal_3_triggered();

    void on_actionVertical_0_triggered();

    void on_actionVertical_1_triggered();

    void on_actionVertical_2_triggered();

    void on_actionHorizontal_Center_Line_triggered();

    void on_actionVertical_Center_Line_triggered();

    void on_actionFull_Black_triggered();

    void on_actionNine_Images_Maker_triggered();

    void on_actionObject_Coordinates_Output_triggered();

    void on_actionNine_Images_Show_triggered();

    void on_actionNew_Workspace_triggered();

    void on_actionOpen_Workspace_triggered();

    void on_actionLeft_Horizontal_triggered();

    void on_actionLeft_Vertical_triggered();

    void on_actionRight_Horizontal_triggered();

    void on_actionRight_Vertical_triggered();

    void on_actionOutput_ALL_Four_triggered();

    void on_actionRestructed_Model_Show_triggered();

    void on_actionOpen_Cameras_triggered();

    void on_actionOpen_Triggered_Cameras_triggered();

    void on_actionClose_Cameras_triggered();

    void on_actionCameras_Viewer_triggered();

    void on_actionProjection_Control_triggered();

    void on_actionProject_and_Capture_triggered();
    void projectAndCaptureAction();

    void on_actionNine_Images_Average_triggered();

    void on_actionNine_Images_Gray_Code_Maker_triggered();

private:
    void projectFringes(FringeType fringeType);

    void phaseMapShow(int phaseNo);

private:
    Ui::MainWindow *ui;

    //Cameras
    ::Camera *camera;
    unsigned char *frmL, *frmR;
    CameraViewer *cameraViewer;
    /*
    QTimer *cameraTimer;
    QPushButton *startButn;
    QPushButton *stopButn;
    QSpinBox *snapshotSpin;
    QPushButton *snapshotButn;
    */

    //Projection
    Projection *projection;

    //Project and Capture
    QTimer *pcTimer;

    NineImagesMaker nineImagesMaker;
    NineWithGrayCodeMaker nineGrayCodeMaker;

    ModelShow *modelShow;
};

#endif // MAINWINDOW_H
