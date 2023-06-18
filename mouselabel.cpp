#include "mouselabel.h"
#include<QPoint>
#include<QPixmap>
#include<QFileDialog>
#include<QException>
int rows, cols;


MouseLabel::MouseLabel(QWidget *parent):QLabel(parent)
{
    this->setMouseTracking(true);
    unsigned int row,col;
    row = this->height();
    col = this->width();
    pixmap = new QPixmap(fname);
    QRgb temp = qRgb(0,0,0);
    pixMat = new QRgb*[row];
    opPixMat = new QRgb*[row];
    for(unsigned int i=0;i<row;i++)
    {
        pixMat[i] = new QRgb[col];
        opPixMat[i] = new QRgb[col];
    }
    for(unsigned int i=0;i<row;i++)
    {
        for(unsigned int j=0;j<col;j++)
        {
            pixMat[i][j] = temp;
            opPixMat[i][j] = temp;
        }
    }
    currentPos.setX(col/2);
    currentPos.setY(row/2);
}


void MouseLabel::paintEvent(QPaintEvent *)
{
    QPainter pa(this);
//    try
//    {
//        pa.drawPixmap(QRect(0,0,300,300),*pixmap);
//    }
//    catch(...)
//    {
        pa.drawPixmap(0,0,*pixmap);
//    }
}
void MouseLabel::mouseMoveEvent(QMouseEvent *ev)
{
    point = ev->pos();
    obName=this->objectName();
    emit sendMousePosition(point,obName);
}

void MouseLabel::mousePressEvent(QMouseEvent *ev)
{
    pixmap = new QPixmap(QPixmap::fromImage(image));
    obName = this->objectName();
    QPoint point;
    point.setX(ev->x());
    point.setY(ev->y());
    if (ev->button() == Qt::RightButton)
        color = color == Qt::red ? Qt::blue : Qt::red;
    else
    {
        pressed = 1;
        drawGrid(ev);
        emit sendClickedPosition(point,obName);
        //draw(ev);
    }
}

void MouseLabel::leaveEvent(QEvent *)
{
    pixmap = new QPixmap(QPixmap::fromImage(image));
    getPosition(currentPos.x(),currentPos.y());
    //repaint();
}
void MouseLabel::getPixmap(QImage &img)
{
    int i,j;
    rows = img.height();
    cols = img.width();
    pixmap = new QPixmap(QPixmap::fromImage(img));
    opimage = *pixmap;
    opPixMat = new QRgb*[rows];
    for(i=0;i<rows;i++)
    {
        opPixMat[i] = new QRgb[cols];
    }
    img=img.convertToFormat(QImage::Format_RGB32);
    image = img;
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            opPixMat[i][j] = img.pixel(i,j);
        }
    }
    repaint();
}

void MouseLabel::getPosition(int x, int y)
{
    currentPos.setX(x);
    currentPos.setY(y);
    pixmap = new QPixmap(QPixmap::fromImage(image));
    QPainter painter(pixmap);
    QColor xCol = Qt::gray;
    QColor yCol = Qt::gray;
    if(this->objectName().compare("lblXY")==0)
    {
        xCol = Qt::magenta;
        yCol = Qt::cyan;
    }
    else if(this->objectName().compare("lblYZ")==0)
    {
        xCol = Qt::cyan;
        yCol = Qt::yellow;
    }
    else
    {
        xCol = Qt::magenta;
        yCol = Qt::yellow;
    }
    painter.setPen(QPen(xCol,1));
    painter.drawLine(x,0,x,this->size().height());
    painter.setPen(QPen(yCol,1));
    painter.drawLine(0,y,this->size().width(),y);
    //qDebug()<<"Inside get position";
    repaint();
}
void MouseLabel::drawGrid(QMouseEvent *ev)
{
    int x = ev->pos().x(),y = ev->pos().y();
    if (pressed) {
        QPainter painter(pixmap);
        painter.setPen(color);

        if(rows<300)
            x = (ev->pos().x())/(300/rows);
        if(cols<300)
            y = (ev->pos().y())/(300/cols);

        painter.setPen(QPen(Qt::red,1));
        painter.drawLine(x,0,x,this->size().height());
        painter.setPen(QPen(Qt::blue,1));
        painter.drawLine(0,y,this->size().width(),y);
        //qDebug()<<"Inside Draw Grid";
        repaint();
    }
}
