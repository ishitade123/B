#ifndef MOUSELABEL_H
#define MOUSELABEL_H

#include<QLabel>
#include<QObject>
#include<QWidget>

#include<QMouseEvent>
#include<QEvent>

#include<QtGui>
#include<QtCore>

class MouseLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MouseLabel(QWidget *parent = 0);
private:
    QPoint point;
    QPoint currentPos;
    QImage image;
    QString fname,obName;
    QColor color;
    QPixmap *pixmap;
    QPixmap opimage;
    QRgb **pixMat;
    QRgb **opPixMat;
    bool pressed;
    void draw(QMouseEvent *e);
    void drawGrid(QMouseEvent *ev);
protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    //void mouseDoubleClickEvent(QMouseEvent *ev);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
signals:
    void sendMousePosition(QPoint&,QString&);
    void seedInfo(QPoint&,QRgb&,QString&);
    void sendClickedPosition(QPoint&,QString&);
public slots:
    void getPixmap(QImage &img);
    void getPosition(int x,int y);
};

#endif // MOUSELABEL_H
