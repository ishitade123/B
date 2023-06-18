#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMouseEvent>
#include<QEvent>
#include<math.h>
#include<random>
#include<time.h>

namespace Ui {
class MainWindow;
}
struct voxl
{
    int x;
    int y;
    int z;
};
typedef struct voxl voxel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    int ctrlX[4],ctrlY[4],ctrlZ[4],dtPoint[4],pointCounter;
    ~MainWindow();
private slots:
    void on_btnLoadImage_clicked();
    void getMousePosition(QPoint &pos,QString &obName);
    //void getSeedInfo(QPoint &pos,QRgb &val,QString &obName);
    void getClickedPosition(QPoint &pos,QString &obName);
    void on_spinBoxX_valueChanged(int arg1);
    void on_spinBoxY_valueChanged(int arg1);
    void on_spinBoxZ_valueChanged(int arg1);
    void on_btnBinarize_clicked();
    void on_btnDistTrans_clicked();
    void on_btnSkeleton_clicked();
    void on_btnWrite_clicked();
    void on_btnAddPoint_clicked();
    void on_btnResetPoints_clicked();
    void on_btnDrawBezier_clicked();
    void on_btnReadSeeds_clicked();
    void on_btnAddNoise_clicked();

private:
    Ui::MainWindow *ui;
    unsigned short Xdim,Ydim,Zdim,CurrentX,CurrentY,CurrentZ;
    const char *filePath;
    QPoint point;
    void *hdr;
    void histogram();
    void readImage();
    unsigned int getDimValue(unsigned char lByte,unsigned char HByte);
    void addnoise();
    void selectnoisevox();
    void drawImage(int type);
    void drawImageXY(int type);
    void drawImageXZ(int type);
    void drawImageYZ(int type);
    void drawsphere(unsigned short***,int,int,int,int,unsigned short);
    void smoothsphere(unsigned short***,int ,int ,int ,int,unsigned short);
    void clearAll();
    void clearMemory(int type);
    unsigned int countSeeds(QString );
    double avg;
signals:
    void sendPixmapXY(QImage &);
    void sendPixmapXZ(QImage &);
    void sendPixmapYZ(QImage &);
    void sendPositionToXY(int x,int y);
    void sendPositionToYZ(int x,int y);
    void sendPositionToXZ(int x,int y);
};

#endif // MAINWINDOW_H
