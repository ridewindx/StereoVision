#ifndef PROJECTION_H
#define PROJECTION_H

#include <QDialog>

extern int projectionWidth;
extern int projectionHeight;

enum FringeType{
    HF0,HF1,HF2,VF0,VF1,VF2,HC,VC,BK,
    WT,
    GCH0,GCH1,GCH2,GCH3,GCH4,GCH5,GCH6,GCH7,GCH8,GCH9,
    GCV0,GCV1,GCV2,GCV3,GCV4,GCV5,GCV6,GCV7,GCV8,GCV9
};

namespace Ui {
class Projection;
}

class Projection : public QDialog
{
    Q_OBJECT

public:
    explicit Projection(QWidget *parent = 0, bool isInteractive=false);
    ~Projection();

    void paintEvent(QPaintEvent *);
    void hideEvent(QHideEvent * event);

    void generateGrayCode();
public slots:
    void setFringeType(int value);

private:
    Ui::Projection *ui;

public:
    int fringeType;

    QDialog *controlDialog;

private:
    int nBits;
    int maxCode;
    int offsetH;
    int offsetV;
    unsigned int *hCode;
    unsigned int *vCode;
};

#endif // PROJECTION_H
