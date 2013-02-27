#include <QtGUI/QApplication>
#include "mainwindow.h"

#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("Stereo Vision");

    return a.exec();
}
