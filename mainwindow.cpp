#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
//#include <windows.h>
#include<cmath>
#include<QThread>
#include<QDebug>
#include<QMessageBox>
#include <bitset>
#include <climits>
#include<vector>
#include<array>
#include<sstream>
using namespace std;
#define d1 1
#define d2 1.4
#define d3 1.7
#define ws 1
#define steps 3
short sixnpp[6][3]={
                               {0,0,-1},

                               {0,-1,0},
                      {-1,0,0},        {+1,0,0},
                               {0,+1,0},

                               {0,0,+1}
};
QString baseFileName,imgFilePath,hdrFilePath,imgFileName;
unsigned short imageType=1,***opImg = NULL,normalIntensity = 100,arteryInten = 210,veinInten = 30, sepInten = 150;
unsigned int ***inputImg = NULL,***binImg = NULL,***dtArray = NULL,***skelImg = NULL,numOfCurves=0,hist[256],pixcount=0;
unsigned short ***b=NULL,pixel,avg1;
short ***noise=NULL,***checknoise=NULL,***avgnoise=NULL;
int op_type=0,binFlag=0,dtFlag = 0,skelFlag = 0,phantomFlag=0,noiseFlag=0,iterations,imgHeight,imgWidth,imgDepth;
QQueue<voxel> AQ,VQ;
bool isEndPoint(int,int,int);
unsigned int* getNeighborhood(int,int,int);
unsigned int getPixelNoCheck(int,int,int);
unsigned int getPixel(int,int,int);
void setPixel(int,int,int,unsigned int);
unsigned int N(int,int,int);
unsigned int S(int,int,int);
unsigned int E(int,int,int);
unsigned int W(int,int,int);
unsigned int U(int,int,int);
unsigned int B(int,int,int);
void fillEulerLUT(int*);
void fillnumOfPointsLUT(int*);
bool isEulerInvariant(unsigned int*,int*);
short indexOctantNEB(unsigned int*);
short indexOctantNWB(unsigned int*);
short indexOctantSEB(unsigned int*);
short indexOctantSWB(unsigned int*);
short indexOctantNEU(unsigned int*);
short indexOctantNWU(unsigned int*);
short indexOctantSEU(unsigned int*);
short indexOctantSWU(unsigned int*);
bool isSimplePoint(unsigned int*);
void octreeLabeling(int,int,int*);
int getThinningIterations();
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Xdim = Ydim = Zdim = pointCounter = 0;
    ui->horizontalSliderX->setEnabled(false);
    ui->horizontalSliderY->setEnabled(false);
    ui->horizontalSliderZ->setEnabled(false);
    ui->spinBoxX->setEnabled(false);
    ui->spinBoxY->setEnabled(false);
    ui->spinBoxZ->setEnabled(false);
    ui->lblXY->setEnabled(false);
    ui->lblYZ->setEnabled(false);
    ui->lblXZ->setEnabled(false);
    ui->btnBinarize->setEnabled(false);
    //ui->btnBinarize->hide();
    //ui->btnSkeleton->hide();
    //ui->btnDistTrans->setEnabled(false);
    ui->btnAddPoint->setEnabled(false);
//    ui->btnAddNoise->setEnabled(false);
    ui->btnDrawBezier->setEnabled(true);
    //ui->btnDrawBezier->setEnabled(false);

    connect(ui->lblXY,SIGNAL(sendMousePosition(QPoint&,QString&)),this,SLOT(getMousePosition(QPoint&,QString&)));
    connect(ui->lblYZ,SIGNAL(sendMousePosition(QPoint&,QString&)),this,SLOT(getMousePosition(QPoint&,QString&)));
    connect(ui->lblXZ,SIGNAL(sendMousePosition(QPoint&,QString&)),this,SLOT(getMousePosition(QPoint&,QString&)));

    connect(this,SIGNAL(sendPixmapXY(QImage&)),ui->lblXY,SLOT(getPixmap(QImage&)));
    connect(this,SIGNAL(sendPixmapYZ(QImage&)),ui->lblYZ,SLOT(getPixmap(QImage&)));
    connect(this,SIGNAL(sendPixmapXZ(QImage&)),ui->lblXZ,SLOT(getPixmap(QImage&)));

    connect(this,SIGNAL(sendPositionToXY(int,int)),ui->lblXY,SLOT(getPosition(int,int)));
    connect(this,SIGNAL(sendPositionToYZ(int,int)),ui->lblYZ,SLOT(getPosition(int,int)));
    connect(this,SIGNAL(sendPositionToXZ(int,int)),ui->lblXZ,SLOT(getPosition(int,int)));

    connect(ui->lblXY,SIGNAL(sendClickedPosition(QPoint&,QString&)),this,SLOT(getClickedPosition(QPoint&,QString&)));
    connect(ui->lblYZ,SIGNAL(sendClickedPosition(QPoint&,QString&)),this,SLOT(getClickedPosition(QPoint&,QString&)));
    connect(ui->lblXZ,SIGNAL(sendClickedPosition(QPoint&,QString&)),this,SLOT(getClickedPosition(QPoint&,QString&)));
    QFile file("coordinates.txt");
    file.remove();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_btnLoadImage_clicked()
{
    hdrFilePath=QFileDialog::getOpenFileName(this,tr("Open File"), "../Data/",tr("Analyze Images (*.hdr)"));
    if(hdrFilePath.compare("")!=0)
    {
        QFileInfo fileInfo(hdrFilePath);
        baseFileName = fileInfo.baseName();
        imgFileName = baseFileName;
        baseFileName.append(".img");
        imgFilePath = fileInfo.path();
        imgFilePath.append("/"+baseFileName);
        op_type = 0;
        /*clearAll();
        clearMemory(op_type);*/

        ui->horizontalSliderX->setEnabled(true);
        ui->horizontalSliderY->setEnabled(true);
        ui->horizontalSliderZ->setEnabled(true);
        ui->spinBoxX->setEnabled(true);
        ui->spinBoxY->setEnabled(true);
        ui->spinBoxZ->setEnabled(true);
        ui->btnBinarize->setEnabled(true);
        ui->btnDistTrans->setEnabled(true);
        //ui->btnHistEqual->setEnabled(true);
        //ui->btnCCl->setEnabled(true);
        //ui->btnReset->setEnabled(true);
        ui->lblXY->setEnabled(true);
        ui->lblXZ->setEnabled(true);
        ui->lblYZ->setEnabled(true);
        histogram();
        ui->lblXY->setMinimumHeight(Ydim);
        ui->lblXY->setMinimumWidth(Xdim);

        ui->lblYZ->setMinimumHeight(Zdim);
        ui->lblYZ->setMinimumWidth(Ydim);

        ui->lblXZ->setMinimumHeight(Xdim);
        ui->lblXZ->setMinimumWidth(Zdim);

        CurrentX = Xdim/2;
        CurrentY = Ydim/2;
        CurrentZ = Zdim/2;

        ui->horizontalSliderX->setRange(0,Xdim-1);
        ui->horizontalSliderY->setRange(0,Ydim-1);
        ui->horizontalSliderZ->setRange(0,Zdim-1);
        ui->spinBoxX->setRange(0,Xdim-1);
        ui->spinBoxY->setRange(0,Ydim-1);
        ui->spinBoxZ->setRange(0,Zdim-1);

        ui->horizontalSliderX->setValue(Xdim/2);
        ui->horizontalSliderY->setValue(Ydim/2);
        ui->horizontalSliderZ->setValue(Zdim/2);
        ui->spinBoxX->setValue(Xdim/2);
        ui->spinBoxY->setValue(Ydim/2);
        ui->spinBoxZ->setValue(Zdim/2);

        readImage();
//        on_btnReadSeeds_clicked();
//        write_image(0);
    }
}
void MainWindow::getMousePosition(QPoint &pos, QString &obName)
{
    if(obName.compare("lblXY")==0)
    {
        ui->lblXCor->setText(QString::number(pos.x()));
        ui->lblYCor->setText(QString::number(pos.y()));

        ui->lblZCor->setText(QString::number(ui->horizontalSliderZ->value()));
    }
    else if(obName.compare("lblYZ")==0)
    {
        ui->lblYCor->setText(QString::number(pos.x()));
        ui->lblZCor->setText(QString::number(pos.y()));

        ui->lblXCor->setText(QString::number(ui->horizontalSliderX->value()));
    }
    else
    {
        ui->lblXCor->setText(QString::number(pos.x()));
        ui->lblZCor->setText(QString::number(pos.y()));

        ui->lblYCor->setText(QString::number(ui->horizontalSliderY->value()));
    }
}
/*void MainWindow::getSeedInfo(QPoint &pos, QRgb &val, QString &obName)
{

}*/
void MainWindow::getClickedPosition(QPoint &pos, QString &obName)
{
    if(obName.compare("lblXY")==0)
    {
        ui->horizontalSliderX->setValue(pos.x());
        ui->horizontalSliderY->setValue(pos.y());
    }
    else if(obName.compare("lblYZ")==0)
    {
        ui->horizontalSliderY->setValue(pos.x());
        ui->horizontalSliderZ->setValue(pos.y());
    }
    else
    {
        ui->horizontalSliderX->setValue(pos.x());
        ui->horizontalSliderZ->setValue(pos.y());
    }
}
void MainWindow::histogram()
{
    unsigned int x,y,z;
    unsigned char lowByte,highByte;
    QFile hdrFile(hdrFilePath);
    QByteArray contents;
    FILE *fo;
    hdr=malloc(480);
    fo = fopen(hdrFilePath.toStdString().c_str(),"rb");
    if(fo)
        fread(hdr,348,1,fo);
    fclose(fo);
    if (!hdrFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,tr("Error"),tr("Unable to open hdr file"));
        //QThread::msleep(1000);
        qApp->exit(0);
    }
    else
    {
        int pointer = 42;
        contents = hdrFile.readAll();
        hdrFile.close();

        lowByte = (unsigned char)contents.at(pointer++);
        highByte = (unsigned char)contents.at(pointer++);
        x = getDimValue(lowByte,highByte);

        lowByte = (unsigned char)contents.at(pointer++);
        highByte = (unsigned char)contents.at(pointer++);
        y = getDimValue(lowByte,highByte);

        lowByte = (unsigned char)contents.at(pointer++);
        highByte = (unsigned char)contents.at(pointer++);
        z = getDimValue(lowByte,highByte);

        Xdim = x;
        Ydim = y;
        Zdim = z;
        imgWidth = x;
        imgHeight = y;
        imgDepth = z;
        qDebug()<<"X: "<<x<<" Y: "<<y<<" Z: "<<z;
        //QThread::msleep(2000);
        inputImg = new unsigned int**[Xdim];
        for(int i = 0;i<Xdim;i++)
        {
            inputImg[i] = new unsigned int*[Ydim];
            for(int j = 0;j<Ydim;j++)
            {
                inputImg[i][j] = new unsigned int[Zdim];
            }
        }
        opImg = new unsigned short**[Xdim];
        for(int i = 0;i<Xdim;i++)
        {
            opImg[i] = new unsigned short*[Ydim];
            for(int j = 0;j<Ydim;j++)
            {
                opImg[i][j] = new unsigned short[Zdim];
            }
        }
        contents.clear();
    }
}
void MainWindow::readImage()
{
    int i,j,k;
    unsigned short pixel;
    unsigned char pixelChar;
    FILE *fp;
    QByteArray ba = imgFilePath.toLatin1();
    filePath = ba.data();
    fp=fopen(filePath,"rb");
    if(!fp)
    {
        QMessageBox::critical(this,tr("Error"),tr("Unable to open img file"));
        QThread::msleep(1000);
        qApp->exit(0);
    }
    if(imageType==1)
    {
        for(k=0;k<Zdim;k++)        // read the image
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    if(fread(&pixel,sizeof(unsigned short),1,fp)==1)
                    {
                        inputImg[i][j][k]=pixel;
                    }
                }
            }
        }
    }
    else
    {
        for(k=0;k<Zdim;k++)        // read the image
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    if(fread(&pixelChar,sizeof(unsigned char),1,fp)==1)
                    {
                        inputImg[i][j][k]=(unsigned short)pixelChar;
                    }
                }
            }
        }
    }
    fclose(fp);
    drawImage(op_type);
    ui->lblOuput->setText("Image loaded successfully");
}
unsigned int MainWindow::getDimValue(unsigned char lByte, unsigned char HByte)
{
    unsigned short dim;
    dim = HByte;
    dim= dim << 8;
    dim = dim | (unsigned short)lByte;
    return (int)dim;
}
void MainWindow::addnoise()
{
    int sum=0,i,j,k;
    std::random_device rd{};
    std::default_random_engine generator;
    //std::uniform_int_distribution<int> uniform_dist(-128,127);
    std::normal_distribution<double> distrN(127.0,12.0);
    for(k=0;k<Zdim/steps;k++)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
               short num=(short)distrN(generator);
               num=num%255;
               noise[i][j][k]=num;
            }
        }
    }
    for(k=0;k<(Zdim/steps);k++)
    {
        for(j=0;j<(Ydim/steps);j++)
        {
            for(int i=0;i<Xdim/steps;i++)
            {
                sum=0;
                for(int r=k-1;r<=k+1;r++)
                {
                    for(int q=j-1;q<=j+1;q++)
                    {
                        for(int p=i-1;p<=i+1;p++)
                        {
                           if(p>=0&&p<Xdim/steps&&q>=0&&q<Ydim/steps&&r>=0&&r<Zdim/steps)
                           {
                                sum=sum+noise[p][q][r];
                           }
                        }
                    }
                }
                double avg=sum/27.0;
                avg=round(avg);
                avgnoise[i][j][k]=(short)avg;
            }
        }
    }
    for(int k=0;k<(Zdim/steps);k++)
    {
        for(int j=0;j<(Ydim/steps);j++)
        {
            for(int i=0;i<(Xdim/steps);i++)
            {
                avgnoise[i][j][k]=avgnoise[i][j][k]-127;
            }
        }
    }
    for(int i=0;i<256;i++)
    {
           hist[i]=0;
    }
    unsigned int count=0;
    for(k=0;k<Zdim/steps;k++)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
                if(b[i][j][k])
                {
                    short value=avgnoise[i][j][k]+(short)b[i][j][k];
                    if(value<=0)
                        value=1;
                    b[i][j][k]=(unsigned short)value;
//                    hist[b[i][j][k]]+=1;
                    count++;
                }
            }
        }
    }
    printf("NonZero Count=%d",count);
//    FILE *fp;
//    fp=fopen("histo_ellipse_withnoise12.csv","wb");
//    if(!fp)
//    {
//        printf("\n Unable to open file");
//        exit(0);
//    }
//    for(int i=0;i<256;i++)
//    {
//           fprintf(fp,"%d,%d\n",i,hist[i]);
//    }
//    fclose(fp);
}
void MainWindow::selectnoisevox()
{
    int i,j,k;
    unsigned int c=0;
    for(k=0;k<Zdim/steps;k++)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
                checknoise[i][j][k]=0;
            }
        }
    }
    srand(time(0));
    while(1)
    {

            int x=rand()%((Xdim/steps)-1);
            int y=rand()%((Ydim/steps)-1);
            int z=rand()%((Zdim/steps)-1);
            if(checknoise[x][y][z]==0)
            {
                short value=avgnoise[x][y][z]+(short)b[x][y][z];
                if(value<=0)value=1;
                b[x][y][z]=(unsigned short)value;
                if(b[x][y][z]>3000)printf("lessthan 127");
                checknoise[x][y][z]++;
                c++;
            }
            if(c>=pixcount/2)break;
    }
    qDebug()<<"Noise added in "<<c<<" number of voxels";
}
void MainWindow::drawImage(int op_type)
{
    drawImageXY(op_type);
    drawImageXZ(op_type);
    drawImageYZ(op_type);
    emit sendPositionToXY(CurrentX,CurrentY);
    emit sendPositionToYZ(CurrentY,CurrentZ);
    emit sendPositionToXZ(CurrentX,CurrentZ);
}
void MainWindow::drawImageXY(int op_type)
{
    int i,j;
    unsigned short defVal;
    QImage opImg_XY(Xdim,Ydim,QImage::Format_RGB32);
    QRgb value;
    if(noiseFlag)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
                defVal = b[i][j][CurrentZ];
                value=qRgb(defVal,defVal,defVal);
                opImg_XY.setPixel(QPoint(i,j),value);
            }
        }
    }
    else
    {
        for(j=0;j<Ydim;j++)
        {
            for(i=0;i<Xdim;i++)
            {
                switch(op_type)
                {
                case 0:defVal=inputImg[i][j][CurrentZ];
                    break;
                case 1:defVal=binImg[i][j][CurrentZ];
                    break;
                case 2:defVal=dtArray[i][j][CurrentZ];
                    break;
                case 3:defVal = skelImg[i][j][CurrentZ];
                    break;
                case 5:defVal = opImg[i][j][CurrentZ];
                    break;
                default:defVal=inputImg[i][j][CurrentZ];
                }
                value=qRgb(defVal,defVal,defVal);
                opImg_XY.setPixel(QPoint(i,j),value);
            }
        }
    }
    emit sendPixmapXY(opImg_XY);
}
void MainWindow::drawImageXZ(int op_type)
{
    int i,j;
    unsigned short defVal;
    QImage opImg_XZ(Xdim,Zdim,QImage::Format_RGB32);
    QRgb value;
    if(noiseFlag)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
                defVal = b[i][CurrentY][j];
                value=qRgb(defVal,defVal,defVal);
                opImg_XZ.setPixel(QPoint(i,j),value);
            }
        }
    }
    else
    {
        for(j=0;j<Zdim;j++)
        {
            for(i=0;i<Xdim;i++)
            {
                switch(op_type)
                {
                case 0:defVal=inputImg[i][CurrentY][j];
                    break;
                case 1:defVal=binImg[i][CurrentY][j];
                    break;
                case 2:defVal=dtArray[i][CurrentY][j];
                    break;
                case 3:defVal = skelImg[i][CurrentY][j];
                    break;
                case 5:defVal = opImg[i][CurrentY][j];
                    break;
                default:defVal=inputImg[i][CurrentY][j];
                }
                value=qRgb(defVal,defVal,defVal);
                opImg_XZ.setPixel(QPoint(i,j),value);
            }
        }
    }
    emit sendPixmapXZ(opImg_XZ);
}
void MainWindow::drawImageYZ(int op_type)
{
    int i,j;
    unsigned short defVal;
    QImage opImg_YZ(Ydim,Zdim,QImage::Format_RGB32);
    QRgb value;
    if(noiseFlag)
    {
        for(j=0;j<Ydim/steps;j++)
        {
            for(i=0;i<Xdim/steps;i++)
            {
                defVal = b[CurrentX][i][j];
                value=qRgb(defVal,defVal,defVal);
                opImg_YZ.setPixel(QPoint(i,j),value);
            }
        }
    }
    else
    {
        for(j=0;j<Zdim;j++)
        {
            for(i=0;i<Ydim;i++)
            {
                switch(op_type)
                {
                case 0:defVal=inputImg[CurrentX][i][j];
                    break;
                case 1:defVal=binImg[CurrentX][i][j];
                    break;
                case 2:defVal=dtArray[CurrentX][i][j];
                    break;
                case 3:defVal = skelImg[CurrentX][i][j];
                    break;
                case 5:defVal = opImg[CurrentX][i][j];
                    break;
                default:defVal=inputImg[CurrentX][i][j];
                }
                value=qRgb(defVal,defVal,defVal);
                opImg_YZ.setPixel(QPoint(i,j),value);
            }
        }
    }
    emit sendPixmapYZ(opImg_YZ);
}
void MainWindow::drawsphere(unsigned short ***a, int r, int xc, int yc, int zc,unsigned short intensity)
{
    //printf("Draw sphere : %d %d %d \n",xc,yc,zc);
    unsigned short *b;
    b = (unsigned short*)calloc(Zdim+1,sizeof(unsigned short));
    int z=0;
    int x=r;
    b[zc]=x;
    b[zc+x]=z;
    b[zc-x]=z;
    int p=1-r;
    while(z<=x)
    {
        if (p<0)
        {
            p=(p+(2*z)+3);
            //x=x;
            z=z+1;
        }
        else
        {
            p=p+(2*(z-x)+5);
            x=x-1;
            z=z+1;
        }
        if(z>0)
        {
            b[zc+z]=x;
            b[zc-z]=x;
            b[zc+x]=z;
            b[zc-x]=z;
        }
    }
    for(int k=zc;k>=zc-r;k--)
    {
        int y=b[k];
        int r1=y;
        for(int c=0;c<=r1;c++)
        {
            int x=0;
            int y=c;
            int p=1-c;
            a[xc+x][yc-y][k]=intensity;
            a[xc-x][yc-y][k]=intensity;
            a[xc+x][yc+y][k]=intensity;
            a[xc-x][yc+y][k]=intensity;
            a[xc+y][yc-x][k]=intensity;
            a[xc-y][yc-x][k]=intensity;
            a[xc+y][yc+x][k]=intensity;
            a[xc-y][yc+x][k]=intensity;

            while(x<=y)
            {
                if (p<=0)
                {
                    p=(p+(2*x)+3);
                    //y=y;
                    x++;
                }
                else
                {
                    p=p+(2*(x-y)+5);
                    y=y-1;
                    x++;
                }
                a[xc+x][yc-y][k]=intensity;
                a[xc-x][yc-y][k]=intensity;
                a[xc+x][yc+y][k]=intensity;
                a[xc-x][yc+y][k]=intensity;
                a[xc+y][yc-x][k]=intensity;
                a[xc-y][yc-x][k]=intensity;
                a[xc+y][yc+x][k]=intensity;
                a[xc-y][yc+x][k]=intensity;
            }
        }
        smoothsphere(a,r1,xc,yc,k,intensity);
        //break;
    }
    for(int k=zc;k<=zc+r;k++)
    {
        int y=b[k];
        int r1=y;
        for(int c=0;c<=r1;c++)
        {
            int x=0;
            int y=c;
            int p=1-c;
            a[xc+x][yc-y][k]=intensity;
            a[xc-x][yc-y][k]=intensity;
            a[xc+x][yc+y][k]=intensity;
            a[xc-x][yc+y][k]=intensity;
            a[xc+y][yc-x][k]=intensity;
            a[xc-y][yc-x][k]=intensity;
            a[xc+y][yc+x][k]=intensity;
            a[xc-y][yc+x][k]=intensity;
            while(x<=y)
            {
                if (p<=0)
                {
                    p=(p+(2*x)+3);
                    //y=y;
                    x++;
                }
                else
                {
                    p=p+(2*(x-y)+5);
                    y=y-1;
                    x++;
                }
                a[xc+x][yc-y][k]=intensity;
                a[xc-x][yc-y][k]=intensity;
                a[xc+x][yc+y][k]=intensity;
                a[xc-x][yc+y][k]=intensity;
                a[xc+y][yc-x][k]=intensity;
                a[xc-y][yc-x][k]=intensity;
                a[xc+y][yc+x][k]=intensity;
                a[xc-y][yc+x][k]=intensity;
            }
        }
        smoothsphere(a,r1,xc,yc,k,intensity);
        //break;
    }
    qDebug()<<"Sphere with Centre at %d %d %d is created: "<<xc<<yc<<zc;
}
void MainWindow::smoothsphere(unsigned short ***a, int r, int xc, int yc, int zc,unsigned short intensity)
{
    int k=zc;
    for(int i=xc-r;i<xc+r;i++)
    {
        for(int j=yc;j<yc+r;j++)
        {
            if(a[i][j][k]!=intensity && a[i][j-1][k]!=0 && a[i-1][j][k]!=0 && a[i+1][j][k]!=0 && a[i][j+1][k]!=0)
            {
                a[i][j][k]=intensity;
            }
        }
    }

    for(int i=xc-r;i<xc+r;i++)
    {
        for(int j=yc;j>=yc-r;j--)
        {
            if(a[i][j][k]!=intensity && a[i][j-1][k]!=0 && a[i-1][j][k]!=0 && a[i+1][j][k]!=0 && a[i][j+1][k]!=0)
            {
                a[i][j][k]=intensity;
            }
        }
    }
}
void MainWindow::clearAll()
{
    qDebug()<<"Clearing memory";
    if(binFlag)
        clearMemory(1);
    if(dtFlag)
        clearMemory(2);
    if(skelFlag)
        clearMemory(3);
    if(noiseFlag)
        clearMemory(4);
    qDebug()<<"Memory cleared";
}
void MainWindow::clearMemory(int op_type)
{
    int i,j;
    if(Xdim>0&&Ydim>0&&op_type==0)
    {
        for(i=0;i<Xdim;i++)
        {
            for(j=0;j<Ydim;j++)
            {
                delete inputImg[i][j];
            }
            delete inputImg[i];
        }
        delete inputImg;
        //inputImg = NULL;
    }
    else if(op_type == 1)
    {
        for(i=0;i<Xdim;i++)
        {
            for(j=0;j<Ydim;j++)
            {
                delete binImg[i][j];
            }
            delete binImg[i];
        }
        delete binImg;
        binFlag = 0;
    }
    else if(op_type == 2)
    {
        for(i=0;i<Xdim;i++)
        {
            for(j=0;j<Ydim;j++)
            {
                delete dtArray[i][j];
            }
            delete dtArray[i];
        }
        delete dtArray;
        dtFlag = 0;
    }
    else if(op_type == 3)
    {
        for(i=0;i<Xdim;i++)
        {
            for(j=0;j<Ydim;j++)
            {
                delete skelImg[i][j];
            }
            delete skelImg[i];
        }
        delete skelImg;
        skelFlag = 0;
    }
    else
    {
        for(i=0;i<Xdim/steps;i++)
        {
            for(j=0;j<Ydim/steps;j++)
            {
                delete b[i][j];
                delete checknoise[i][j];
                delete noise[i][j];
                delete avgnoise[i][j];
            }
            delete b[i];
            delete checknoise[i];
            delete noise[i];
            delete avgnoise[i];
        }
        delete b;
        delete checknoise;
        delete noise;
        delete avgnoise;
        noiseFlag = 0;
    }
}
unsigned int MainWindow::countSeeds(QString fileName)
{
    unsigned int numOfLines=0;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString line;
        QTextStream inFile(&file);
        while(!inFile.atEnd())
        {
            line = inFile.readLine();
            numOfLines++;
        }
    }
    file.close();
    return numOfLines;
}
void MainWindow::on_spinBoxX_valueChanged(int arg1)
{
    CurrentX = arg1;
    if(phantomFlag)
        ui->lblIntenVal->setText(QString::number(opImg[arg1][CurrentY][CurrentZ]));
    else if(noiseFlag)
        ui->lblIntenVal->setText(QString::number(b[arg1][CurrentY][CurrentZ]));
    else
        ui->lblIntenVal->setText(QString::number(inputImg[arg1][CurrentY][CurrentZ]));
    drawImageYZ(op_type);
    emit sendPositionToXY(CurrentX,CurrentY);
    emit sendPositionToXZ(CurrentX,CurrentZ);
    emit sendPositionToYZ(CurrentY,CurrentZ);
}
void MainWindow::on_spinBoxY_valueChanged(int arg1)
{
    CurrentY = arg1;
    if(phantomFlag)
        ui->lblIntenVal->setText(QString::number(opImg[CurrentX][arg1][CurrentZ]));
    else if(noiseFlag)
        ui->lblIntenVal->setText(QString::number(b[CurrentX][arg1][CurrentZ]));
    else
        ui->lblIntenVal->setText(QString::number(inputImg[CurrentX][arg1][CurrentZ]));
    drawImageXZ(op_type);
    emit sendPositionToXY(CurrentX,CurrentY);
    emit sendPositionToXZ(CurrentX,CurrentZ);
    emit sendPositionToYZ(CurrentY,CurrentZ);
}
void MainWindow::on_spinBoxZ_valueChanged(int arg1)
{
    CurrentZ = arg1;
    if(phantomFlag)
        ui->lblIntenVal->setText(QString::number(opImg[CurrentX][CurrentY][arg1]));
    else if(noiseFlag)
        ui->lblIntenVal->setText(QString::number(b[CurrentX][CurrentY][arg1]));
    else
        ui->lblIntenVal->setText(QString::number(inputImg[CurrentX][CurrentY][arg1]));
    drawImageXY(op_type);
    emit sendPositionToXY(CurrentX,CurrentY);
    emit sendPositionToXZ(CurrentX,CurrentZ);
    emit sendPositionToYZ(CurrentY,CurrentZ);
}
void MainWindow::on_btnBinarize_clicked()
{
    int i,j,k;
    op_type=1;
    clearAll();
    binImg = new unsigned int**[Xdim];
    for(i = 0;i<Xdim;i++)
    {
        binImg[i] = new unsigned int*[Ydim];
        for(j = 0;j<Ydim;j++)
        {
            binImg[i][j] = new unsigned int[Zdim];
        }
    }
    for(k=0;k<Zdim;k++)
    {
        for(j=0;j<Ydim;j++)
        {
            for(i=0;i<Xdim;i++)
            {
                if(inputImg[i][j][k]!=0)
                    binImg[i][j][k]=255;
                else
                    binImg[i][j][k]=0;
            }
        }
    }
    drawImage(op_type);
    binFlag = 1;
}
void MainWindow::on_btnDistTrans_clicked()
{
    int i,j,k,flag1;
    unsigned int min;
    double v;
    unsigned short maxintendt=0;
    clearAll();

    dtArray = new unsigned int**[Xdim];
    for(i = 0;i<Xdim;i++)
    {
        dtArray[i] = new unsigned int*[Ydim];
        for(j = 0;j<Ydim;j++)
        {
            dtArray[i][j] = new unsigned int[Zdim];
        }
    }

    for(k=0;k<Zdim;k++)
    {
        for(j=0;j<Ydim;j++)
        {
            for(i=0;i<Xdim;i++)
            {
                if(inputImg[i][j][k]!=0)
                    dtArray[i][j][k]=65500;
                else
                    dtArray[i][j][k]=0;
            }
        }
    }
    do{
        flag1=0;
        for(k=0;k<Zdim;k++)   //forward pass
            {
                for(j=0;j<Ydim;j++)
                {
                    for(i=0;i<Xdim;i++)
                    {
                        if(dtArray[i][j][k]!=0)
                        {
                            min=dtArray[i][j][k];
                            if((i-1>=0)&&((dtArray[i-1][j][k]+d1)<min))
                            {
                                min=dtArray[i-1][j][k]+d1;
                                flag1++;
                            }
                            if((i-1>=0)&&(j-1>=0)&&((dtArray[i-1][j-1][k]+d2)<min))
                            {
                                min=dtArray[i-1][j-1][k]+d2;
                                flag1++;
                            }
                            if((j-1>=0)&&((dtArray[i][j-1][k]+d1)<min))
                            {
                                min=dtArray[i][j-1][k]+d1;
                                flag1++;
                            }
                            if((i+1<Xdim)&&(j-1>=0)&&((dtArray[i+1][j-1][k]+d2)<min))
                            {
                                min=dtArray[i+1][j-1][k]+d2;
                                flag1++;
                            }
                            if((k+1<Zdim)&&((dtArray[i][j][k+1]+d1)<min))
                            {
                               min=dtArray[i][j][k+1]+d1;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j-1>=0)&&((dtArray[i][j-1][k+1]+d2)<min))
                            {
                               min=dtArray[i][j-1][k+1]+d2;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j+1<Ydim)&&((dtArray[i][j+1][k+1]+d2)<min))
                            {
                               min=dtArray[i][j+1][k+1]+d2;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(i-1>=0)&&((dtArray[i-1][j][k+1]+d2)<min))
                            {
                               min=dtArray[i-1][j][k+1]+d2;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j-1>=0)&&(i-1>=0)&&((dtArray[i-1][j-1][k+1]+d3)<min))
                            {
                               min=dtArray[i-1][j-1][k+1]+d3;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j+1<Ydim)&&(i-1>=0)&&((dtArray[i-1][j+1][k+1]+d3)<min))
                            {
                               min=dtArray[i-1][j+1][k+1]+d3;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(i+1<Xdim)&&((dtArray[i+1][j][k+1]+d2)<min))
                            {
                               min=dtArray[i+1][j][k+1]+d2;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j-1>=0)&&(i+1<Xdim)&&((dtArray[i+1][j-1][k+1]+d3)<min))
                            {
                               min=dtArray[i+1][j-1][k+1]+d3;
                               flag1++;
                            }
                            if((k+1<Zdim)&&(j+1<Ydim)&&(i+1<Xdim)&&((dtArray[i+1][j+1][k+1]+d3)<min))
                            {
                               min=dtArray[i+1][j+1][k+1]+d3;
                               flag1++;
                            }

                            dtArray[i][j][k]=min;
                            if(min>maxintendt)maxintendt=min;
                        }
                    }
                }
            }
            for(k=Zdim-1;k>=0;k--)         //Backward Pass
            {
                for(j=Ydim-1;j>=0;j--)
                {
                    for(i=Xdim-1;i>=0;i--)
                    {
                        if(dtArray[i][j][k]!=0)
                        {
                            min=dtArray[i][j][k];
                            if((i+1<Xdim)&&((dtArray[i+1][j][k]+d1)<min))
                            {
                                min=dtArray[i+1][j][k]+d1;
                                flag1++;
                            }
                            if((j+1<Ydim)&&((dtArray[i][j+1][k]+d1)<min))
                            {
                                min=dtArray[i][j+1][k]+d1;
                                flag1++;
                            }
                            if((i-1>=0)&&(j+1<Ydim)&&((dtArray[i-1][j+1][k]+d2)<min))
                            {
                                min=dtArray[i-1][j+1][k]+d2;
                                flag1++;
                            }
                            if((i+1<Xdim)&&(j+1<Ydim)&&((dtArray[i+1][j+1][k]+d2)<min))
                            {
                                min=dtArray[i+1][j+1][k]+d2;
                                flag1++;
                            }
                            if((k-1>=0)&&((dtArray[i][j][k-1]+d1)<min))
                            {
                                min=dtArray[i][j][k-1]+d1;
                                flag1++;
                            }
                            if((k-1>=0)&&(i-1>=0)&&((dtArray[i-1][j][k-1]+d2)<min))
                            {
                                min=dtArray[i-1][j][k-1]+d2;
                                flag1++;
                            }
                            if((k-1>=0)&&(i+1<Xdim)&&((dtArray[i+1][j][k-1]+d2)<min))
                            {
                                min=dtArray[i+1][j][k-1]+d2;
                                flag1++;
                            }
                            if((k-1>=0)&&(i-1>=0)&&(j-1>=0)&&((dtArray[i-1][j-1][k-1]+d3)<min))
                            {
                                min=dtArray[i-1][j-1][k-1]+d3;
                                flag1++;
                            }
                            if((k-1>=0)&&(j-1>=0)&&((dtArray[i][j-1][k-1]+d2)<min))
                            {
                                min=dtArray[i][j-1][k-1]+d2;
                                flag1++;
                            }
                            if((k-1>=0)&&(i+1<Xdim)&&(j-1>=0)&&((dtArray[i+1][j-1][k-1]+d3)<min))
                            {
                                min=dtArray[i+1][j-1][k-1]+d3;
                                flag1++;
                            }
                            if((k-1>=0)&&(j+1<Ydim)&&((dtArray[i][j+1][k-1]+d2)<min))
                            {
                                min=dtArray[i][j+1][k-1]+d2;
                                flag1++;
                            }
                            if((k-1>=0)&&(i-1>=0)&&(j+1<Ydim)&&((dtArray[i-1][j+1][k-1]+d3)<min))
                            {
                                min=dtArray[i-1][j+1][k-1]+d3;
                                flag1++;
                            }
                            if((k-1>=0)&&(j+1<Ydim)&&(i+1<Xdim)&&((dtArray[i+1][j+1][k-1]+d3)<min))
                            {
                                min=dtArray[i+1][j+1][k-1]+d3;
                                flag1++;
                            }
                            dtArray[i][j][k]=min;
                            if(min>maxintendt)maxintendt=min;
                        }
                    }
                }
            }
    }while(flag1!=0);
    for(k=0;k<Zdim;k++)
    {
        for(j=0;j<Ydim;j++)
        {
            for(i=0;i<Xdim;i++)
            {
                        v=round((dtArray[i][j][k]/(double)maxintendt)*255);
                        dtArray[i][j][k]=v;
            }
        }
    }
    op_type = 2;
    drawImage(op_type);
    dtFlag = 1;
    ui->lblOuput->setText("Distance Transform computation complete");
    ui->btnAddPoint->setEnabled(true);
}
void MainWindow::on_btnSkeleton_clicked()
{
    op_type = 3;
    clearAll();
    int width = Xdim,height = Ydim,depth = Zdim;
    int i,j;
    skelImg = new unsigned int**[Xdim];
    for(i = 0;i<Xdim;i++)
    {
        skelImg[i] = new unsigned int*[Ydim];
        for(j = 0;j<Ydim;j++)
        {
            skelImg[i][j] = new unsigned int[Zdim];
        }
    }
    //Preparing Data for computation
    for(int z = 0;z<depth;z++)
    {
        for(int x = 0;x<width;x++)
        {
            for(int y=0;y<height;y++)
            {
                skelImg[x][y][z]=0;
                if(inputImg[x][y][z]!=0)
                    skelImg[x][y][z]=1;
            }
        }
    }
    int *eulerLUT = new int[256];
    fillEulerLUT( eulerLUT );

    // Prepare number of points LUT
    int *pointsLUT = new int[ 256 ];
    fillnumOfPointsLUT(pointsLUT);

    // Following Lee[94], save versions (Q) of input image S, while
    // deleting each type of border points (R)
    vector<array<int,3>> simpleBorderPoints;
    iterations = 0;
    // Loop through the image several times until there is no change.
    int unchangedBorders = 0;
    while( unchangedBorders < 6 )  // loop until no change for all the six border types
    {
        unchangedBorders = 0;
        iterations++;
        for( int currentBorder = 1; currentBorder <= 6; currentBorder++)
        {
            qDebug()<<"Thinning iteration " + QString::number(iterations) + " (" + QString::number(currentBorder) +"/6 borders) ...";

            bool noChange = true;

            // Loop through the image.
            for (int z = 0; z < depth; z++)
            {
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        // check if point is foreground
                        if ( getPixelNoCheck( x, y, z) != 1 )
                        {
                            continue;         // current point is already background
                        }

                        // check 6-neighbors if point is a border point of type currentBorder
                        bool isBorderPoint = false;
                        // North
                        if( currentBorder == 1 && N( x, y, z) <= 0 )
                            isBorderPoint = true;
                        // South
                        if( currentBorder == 2 && S( x, y, z) <= 0 )
                            isBorderPoint = true;
                        // East
                        if( currentBorder == 3 && E( x, y, z) <= 0 )
                            isBorderPoint = true;
                        // West
                        if( currentBorder == 4 && W( x, y, z) <= 0 )
                            isBorderPoint = true;
                        if(sizeof(skelImg) > 1)
                        {
                            // Up
                            if( currentBorder == 5 && U( x, y, z) <= 0 )
                                isBorderPoint = true;
                            // Bottom
                            if( currentBorder == 6 && B( x, y, z) <= 0 )
                                isBorderPoint = true;
                        }
                        if( !isBorderPoint )
                        {
                            continue;         // current point is not deletable
                        }

                        if( isEndPoint( x, y, z))
                        {
                            continue;
                        }

                        unsigned int* neighborhood = getNeighborhood(x, y, z);

                        // Check if point is Euler invariant (condition 1 in Lee[94])
                        if( !isEulerInvariant( neighborhood, eulerLUT ) )
                        {
                            continue;         // current point is not deletable
                        }

                        // Check if point is simple (deletion does not change connectivity in the 3x3x3 neighborhood)
                        // (conditions 2 and 3 in Lee[94])
                        if( !isSimplePoint( neighborhood ) )
                        {
                            continue;         // current point is not deletable
                        }

                        // add all simple border points to a list for sequential re-checking
                        /*int *index = new int[3];
                        index[0] = x;
                        index[1] = y;
                        index[2] = z;*/
                        array<int,3> index;
                        index[0] = x;
                        index[1] = y;
                        index[2] = z;
                        simpleBorderPoints.push_back(index);
                    }
                }
                //qDebug()<<"Slice: "+QString::number(z)+" of "+QString::number(depth);
            }


            // sequential re-checking to preserve connectivity when
            // deleting in a parallel way
            array <int,3> index;
            vector<array<int,3>>::iterator simpleBorderPoint;
            for (simpleBorderPoint = simpleBorderPoints.begin(); simpleBorderPoint!= simpleBorderPoints.end();simpleBorderPoint++)
            {
                index = *simpleBorderPoint;
                // Check if border points is simple
                if (isSimplePoint(getNeighborhood(index[0], index[1], index[2]))) {
                    // we can delete the current point
                    setPixel(index[0], index[1], index[2], (unsigned int) 0);
                    noChange = false;
                }
            }

            if( noChange )
                unchangedBorders++;

            simpleBorderPoints.clear();
        } // end currentBorder for loop
    }
    for(int z = 0;z<depth;z++)
    {
        for(int x = 0;x<width;x++)
        {
            for(int y=0;y<height;y++)
            {
                skelImg[x][y][z]=skelImg[x][y][z]*100;
            }
        }
    }
    qDebug()<<"Skeletonization complete";
    drawImage(op_type);
}
/**
     * Check if a point in the given stack is at the end of an arc
     *
     * @param image	The stack of a 3D binary image
     * @param x		The x-coordinate of the point
     * @param y		The y-coordinate of the point
     * @param z		The z-coordinate of the point (>= 1)
     * @return		true if the point has exactly one neighbor
     */
bool isEndPoint(int x,int y,int z)
{
    int numberOfNeighbors = -1;   // -1 and not 0 because the center pixel will be counted as well
    unsigned int* neighbor = getNeighborhood(x, y, z);
    for( int i = 0; i < 27; i++ ) // i =  0..26
    {
        if( neighbor[i] == 1 )
            numberOfNeighbors++;
    }
    return numberOfNeighbors == 1;
}
/* -----------------------------------------------------------------------*/
/**
     * Get neighborhood of a pixel in a 3D image (0 border conditions)
     *
     * @param image 3D image (ImageStack)
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding 27-pixels neighborhood (0 if out of image)
     */
unsigned int* getNeighborhood(int x, int y, int z)
{
    unsigned int* neighborhood = new unsigned int[27];

    neighborhood[ 0] = getPixel( x-1, y-1, z-1);
    neighborhood[ 1] = getPixel( x  , y-1, z-1);
    neighborhood[ 2] = getPixel( x+1, y-1, z-1);

    neighborhood[ 3] = getPixel( x-1, y,   z-1);
    neighborhood[ 4] = getPixel( x,   y,   z-1);
    neighborhood[ 5] = getPixel( x+1, y,   z-1);

    neighborhood[ 6] = getPixel( x-1, y+1, z-1);
    neighborhood[ 7] = getPixel( x,   y+1, z-1);
    neighborhood[ 8] = getPixel( x+1, y+1, z-1);

    neighborhood[ 9] = getPixel( x-1, y-1, z  );
    neighborhood[10] = getPixel( x,   y-1, z  );
    neighborhood[11] = getPixel( x+1, y-1, z  );

    neighborhood[12] = getPixel( x-1, y,   z  );
    neighborhood[13] = getPixel( x,   y,   z  );
    neighborhood[14] = getPixel( x+1, y,   z  );

    neighborhood[15] = getPixel( x-1, y+1, z  );
    neighborhood[16] = getPixel( x,   y+1, z  );
    neighborhood[17] = getPixel( x+1, y+1, z  );

    neighborhood[18] = getPixel( x-1, y-1, z+1);
    neighborhood[19] = getPixel( x,   y-1, z+1);
    neighborhood[20] = getPixel( x+1, y-1, z+1);

    neighborhood[21] = getPixel( x-1, y,   z+1);
    neighborhood[22] = getPixel( x,   y,   z+1);
    neighborhood[23] = getPixel( x+1, y,   z+1);

    neighborhood[24] = getPixel( x-1, y+1, z+1);
    neighborhood[25] = getPixel( x,   y+1, z+1);
    neighborhood[26] = getPixel( x+1, y+1, z+1);

    return neighborhood;
} /* end getNeighborhood */
/* -----------------------------------------------------------------------*/
/**
     * Get pixel in 3D image (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding pixel (0 if out of image)
     */
unsigned int getPixel(int x, int y, int z)
{
    unsigned int voxVal=0;
    if(x >= 0 && x < imgWidth && y >= 0 && y < imgHeight && z >= 0 && z < imgDepth)
        voxVal = skelImg[x][y][z];
    return voxVal;
} /* end getPixel */
unsigned int getPixelNoCheck(int x, int y, int z)
{
    return skelImg[x][y][z];
}/* end getPixelNocheck */
/* -----------------------------------------------------------------------*/
/**
 * Set pixel in 3D image
 *
 * @param image 3D image
 * @param x x- coordinate
 * @param y y- coordinate
 * @param z z- coordinate (in image stacks the indexes start at 1)
 * @param value pixel value
 */
void setPixel(int x, int y, int z, unsigned int value)
{
    if(x >= 0 && x < imgWidth && y >= 0 && y < imgHeight && z >= 0 && z < imgDepth)
        skelImg[x][y][z] = value;
} /* end setPixel */
/* -----------------------------------------------------------------------*/
/**
     * North neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding north pixel
     */
unsigned int N(int x, int y, int z)
{
    return getPixel(x, y-1, z);
} /* end N */
/* -----------------------------------------------------------------------*/
/**
     * South neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding south pixel
     */
unsigned int S(int x, int y, int z)
{
    return getPixel( x, y+1, z);
} /* end S */

/* -----------------------------------------------------------------------*/
/**
     * East neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding east pixel
     */
unsigned int E(int x, int y, int z)
{
    return getPixel(x+1, y, z);
} /* end E */

/* -----------------------------------------------------------------------*/
/**
     * West neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding west pixel
     */
unsigned int W(int x, int y, int z)
{
    return getPixel( x-1, y, z);
} /* end W */

/* -----------------------------------------------------------------------*/
/**
     * Up neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding up pixel
     */
unsigned int U(int x, int y, int z)
{
    return getPixel( x, y, z+1);
} /* end U */

/* -----------------------------------------------------------------------*/
/**
     * Bottom neighborhood (0 border conditions)
     *
     * @param image 3D image
     * @param x x- coordinate
     * @param y y- coordinate
     * @param z z- coordinate (in image stacks the indexes start at 1)
     * @return corresponding bottom pixel
     */
unsigned int B(int x, int y, int z)
{
    return getPixel( x, y, z-1);
} /* end B */
/* -----------------------------------------------------------------------*/
/**
     * Fill Euler LUT
     *
     * @param LUT Euler LUT
     */
void fillEulerLUT(int *LUT)
{
    LUT[1]  =  1;
    LUT[3]  = -1;
    LUT[5]  = -1;
    LUT[7]  =  1;
    LUT[9]  = -3;
    LUT[11] = -1;
    LUT[13] = -1;
    LUT[15] =  1;
    LUT[17] = -1;
    LUT[19] =  1;
    LUT[21] =  1;
    LUT[23] = -1;
    LUT[25] =  3;
    LUT[27] =  1;
    LUT[29] =  1;
    LUT[31] = -1;
    LUT[33] = -3;
    LUT[35] = -1;
    LUT[37] =  3;
    LUT[39] =  1;
    LUT[41] =  1;
    LUT[43] = -1;
    LUT[45] =  3;
    LUT[47] =  1;
    LUT[49] = -1;
    LUT[51] =  1;

    LUT[53] =  1;
    LUT[55] = -1;
    LUT[57] =  3;
    LUT[59] =  1;
    LUT[61] =  1;
    LUT[63] = -1;
    LUT[65] = -3;
    LUT[67] =  3;
    LUT[69] = -1;
    LUT[71] =  1;
    LUT[73] =  1;
    LUT[75] =  3;
    LUT[77] = -1;
    LUT[79] =  1;
    LUT[81] = -1;
    LUT[83] =  1;
    LUT[85] =  1;
    LUT[87] = -1;
    LUT[89] =  3;
    LUT[91] =  1;
    LUT[93] =  1;
    LUT[95] = -1;
    LUT[97] =  1;
    LUT[99] =  3;
    LUT[101] =  3;
    LUT[103] =  1;

    LUT[105] =  5;
    LUT[107] =  3;
    LUT[109] =  3;
    LUT[111] =  1;
    LUT[113] = -1;
    LUT[115] =  1;
    LUT[117] =  1;
    LUT[119] = -1;
    LUT[121] =  3;
    LUT[123] =  1;
    LUT[125] =  1;
    LUT[127] = -1;
    LUT[129] = -7;
    LUT[131] = -1;
    LUT[133] = -1;
    LUT[135] =  1;
    LUT[137] = -3;
    LUT[139] = -1;
    LUT[141] = -1;
    LUT[143] =  1;
    LUT[145] = -1;
    LUT[147] =  1;
    LUT[149] =  1;
    LUT[151] = -1;
    LUT[153] =  3;
    LUT[155] =  1;

    LUT[157] =  1;
    LUT[159] = -1;
    LUT[161] = -3;
    LUT[163] = -1;
    LUT[165] =  3;
    LUT[167] =  1;
    LUT[169] =  1;
    LUT[171] = -1;
    LUT[173] =  3;
    LUT[175] =  1;
    LUT[177] = -1;
    LUT[179] =  1;
    LUT[181] =  1;
    LUT[183] = -1;
    LUT[185] =  3;
    LUT[187] =  1;
    LUT[189] =  1;
    LUT[191] = -1;
    LUT[193] = -3;
    LUT[195] =  3;
    LUT[197] = -1;
    LUT[199] =  1;
    LUT[201] =  1;
    LUT[203] =  3;
    LUT[205] = -1;
    LUT[207] =  1;

    LUT[209] = -1;
    LUT[211] =  1;
    LUT[213] =  1;
    LUT[215] = -1;
    LUT[217] =  3;
    LUT[219] =  1;
    LUT[221] =  1;
    LUT[223] = -1;
    LUT[225] =  1;
    LUT[227] =  3;
    LUT[229] =  3;
    LUT[231] =  1;
    LUT[233] =  5;
    LUT[235] =  3;
    LUT[237] =  3;
    LUT[239] =  1;
    LUT[241] = -1;
    LUT[243] =  1;
    LUT[245] =  1;
    LUT[247] = -1;
    LUT[249] =  3;
    LUT[251] =  1;
    LUT[253] =  1;
    LUT[255] = -1;
}
size_t popcount(unsigned int n) {
  return std::bitset<CHAR_BIT * sizeof n>(n).count();
}
/* -----------------------------------------------------------------------*/
/**
     * Fill number of points in octant LUT
     *
     * @param LUT number of points in octant LUT
     */
void fillnumOfPointsLUT(int* LUT)
{
    for(int i=0; i<256; i++)
        LUT[i] = popcount(i);
}
/**
     * Check if a point is Euler invariant
     *
     * @param neighbors neighbor pixels of the point
     * @param LUT Euler LUT
     * @return true or false if the point is Euler invariant or not
     */
bool isEulerInvariant(unsigned int *neighbors, int *LUT)
{
    // Calculate Euler characteristic for each octant and sum up
    int eulerChar = 0;
    short v;
    // Octant SWU
    v = indexOctantSWU(neighbors);
    eulerChar += LUT[v];

    // Octant SEU
    v = indexOctantSEU(neighbors);
    eulerChar += LUT[v];

    // Octant NWU
    v = indexOctantNWU(neighbors);
    eulerChar += LUT[v];

    // Octant NEU
    v = indexOctantNEU(neighbors);
    eulerChar += LUT[v];

    // Octant SWB
    v = indexOctantSWB(neighbors);
    eulerChar += LUT[v];

    // Octant SEB
    v = indexOctantSEB(neighbors);
    eulerChar += LUT[v];

    // Octant NWB
    v = indexOctantNWB(neighbors);
    eulerChar += LUT[v];

    // Octant NEB
    v = indexOctantNEB(neighbors);
    eulerChar += LUT[v];

    return eulerChar == 0;
}
short indexOctantNEB(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[2]==1 )
        n |= 128;
    if( neighbors[1]==1 )
        n |=  64;
    if( neighbors[11]==1 )
        n |=  32;
    if( neighbors[10]==1 )
        n |=  16;
    if( neighbors[5]==1 )
        n |=   8;
    if( neighbors[4]==1 )
        n |=   4;
    if( neighbors[14]==1 )
        n |=   2;
    return n;
}
short indexOctantNWB(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[0]==1 )
        n |= 128;
    if( neighbors[9]==1 )
        n |=  64;
    if( neighbors[3]==1 )
        n |=  32;
    if( neighbors[12]==1 )
        n |=  16;
    if( neighbors[1]==1 )
        n |=   8;
    if( neighbors[10]==1 )
        n |=   4;
    if( neighbors[4]==1 )
        n |=   2;
    return n;
}
short indexOctantSEB(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[8]==1 )
        n |= 128;
    if( neighbors[7]==1 )
        n |=  64;
    if( neighbors[17]==1 )
        n |=  32;
    if( neighbors[16]==1 )
        n |=  16;
    if( neighbors[5]==1 )
        n |=   8;
    if( neighbors[4]==1 )
        n |=   4;
    if( neighbors[14]==1 )
        n |=   2;
    return n;
}
short indexOctantSWB(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[6]==1 )
        n |= 128;
    if( neighbors[15]==1 )
        n |=  64;
    if( neighbors[7]==1 )
        n |=  32;
    if( neighbors[16]==1 )
        n |=  16;
    if( neighbors[3]==1 )
        n |=   8;
    if( neighbors[12]==1 )
        n |=   4;
    if( neighbors[4]==1 )
        n |=   2;
    return n;
}
short indexOctantNEU(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[20]==1 )
        n |= 128;
    if( neighbors[23]==1 )
        n |=  64;
    if( neighbors[19]==1 )
        n |=  32;
    if( neighbors[22]==1 )
        n |=  16;
    if( neighbors[11]==1 )
        n |=   8;
    if( neighbors[14]==1 )
        n |=   4;
    if( neighbors[10]==1 )
        n |=   2;
    return n;
}
short indexOctantNWU(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[18]==1 )
        n |= 128;
    if( neighbors[21]==1 )
        n |=  64;
    if( neighbors[9]==1 )
        n |=  32;
    if( neighbors[12]==1 )
        n |=  16;
    if( neighbors[19]==1 )
        n |=   8;
    if( neighbors[22]==1 )
        n |=   4;
    if( neighbors[10]==1 )
        n |=   2;
    return n;
}
short indexOctantSEU(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[26]==1 )
        n |= 128;
    if( neighbors[23]==1 )
        n |=  64;
    if( neighbors[17]==1 )
        n |=  32;
    if( neighbors[14]==1 )
        n |=  16;
    if( neighbors[25]==1 )
        n |=   8;
    if( neighbors[22]==1 )
        n |=   4;
    if( neighbors[16]==1 )
        n |=   2;
    return n;
}
short indexOctantSWU(unsigned int *neighbors)
{
    short n;
    n = 1;
    if( neighbors[24]==1 )
        n |= 128;
    if( neighbors[25]==1 )
        n |=  64;
    if( neighbors[15]==1 )
        n |=  32;
    if( neighbors[16]==1 )
        n |=  16;
    if( neighbors[21]==1 )
        n |=   8;
    if( neighbors[22]==1 )
        n |=   4;
    if( neighbors[12]==1 )
        n |=   2;
    return n;
}
/* -----------------------------------------------------------------------*/
/**
     * Check if current point is a Simple Point.
     * This method is named 'N(v)_labeling' in [Lee94].
     * Outputs the number of connected objects in a neighborhood of a point
     * after this point would have been removed.
     *
     * @param neighbors neighbor pixels of the point
     * @return true or false if the point is simple or not
     */
bool isSimplePoint(unsigned int *neighbors)
{
    // copy neighbors for labeling
    int *cube = new int[26];
    int i;
    for( i = 0; i < 13; i++ )  // i =  0..12 -> cube[0..12]
        cube[i] = neighbors[i];
    // i != 13 : ignore center pixel when counting (see [Lee94])
    for( i = 14; i < 27; i++ ) // i = 14..26 -> cube[13..25]
        cube[i-1] = neighbors[i];
    // set initial label
    int label = 2;
    // for all points in the neighborhood
    for( i = 0; i < 26; i++ )
    {
        if( cube[i]==1 )     // voxel has not been labeled yet
        {
            // start recursion with any octant that contains the point i
            switch( i )
            {
            case 0:
            case 1:
            case 3:
            case 4:
            case 9:
            case 10:
            case 12:
                octreeLabeling(1, label, cube );
                break;
            case 2:
            case 5:
            case 11:
            case 13:
                octreeLabeling(2, label, cube );
                break;
            case 6:
            case 7:
            case 14:
            case 15:
                octreeLabeling(3, label, cube );
                break;
            case 8:
            case 16:
                octreeLabeling(4, label, cube );
                break;
            case 17:
            case 18:
            case 20:
            case 21:
                octreeLabeling(5, label, cube );
                break;
            case 19:
            case 22:
                octreeLabeling(6, label, cube );
                break;
            case 23:
            case 24:
                octreeLabeling(7, label, cube );
                break;
            case 25:
                octreeLabeling(8, label, cube );
                break;
            }
            label++;
            if( label-2 >= 2 )
            {
                return false;
            }
        }
    }
    //return label-2; in [Lee94] if the number of connected components would be needed
    return true;
}
/* -----------------------------------------------------------------------*/
/**
     * This is a recursive method that calculates the number of connected
     * components in the 3D neighborhood after the center pixel would
     * have been removed.
     *
     * @param octant
     * @param label
     * @param cube
     */
void octreeLabeling(int octant, int label, int *cube)
{
    // check if there are points in the octant with value 1
    if( octant==1 )
    {
        // set points in this octant to current label
        // and recursive labeling of adjacent octants
        if( cube[0] == 1 )
            cube[0] = label;
        if( cube[1] == 1 )
        {
            cube[1] = label;
            octreeLabeling( 2, label, cube);
        }
        if( cube[3] == 1 )
        {
            cube[3] = label;
            octreeLabeling( 3, label, cube);
        }
        if( cube[4] == 1 )
        {
            cube[4] = label;
            octreeLabeling( 2, label, cube);
            octreeLabeling( 3, label, cube);
            octreeLabeling( 4, label, cube);
        }
        if( cube[9] == 1 )
        {
            cube[9] = label;
            octreeLabeling( 5, label, cube);
        }
        if( cube[10] == 1 )
        {
            cube[10] = label;
            octreeLabeling( 2, label, cube);
            octreeLabeling( 5, label, cube);
            octreeLabeling( 6, label, cube);
        }
        if( cube[12] == 1 )
        {
            cube[12] = label;
            octreeLabeling( 3, label, cube);
            octreeLabeling( 5, label, cube);
            octreeLabeling( 7, label, cube);
        }
    }
    if( octant==2 )
    {
        if( cube[1] == 1 )
        {
            cube[1] = label;
            octreeLabeling( 1, label, cube);
        }
        if( cube[4] == 1 )
        {
            cube[4] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 3, label, cube);
            octreeLabeling( 4, label, cube);
        }
        if( cube[10] == 1 )
        {
            cube[10] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 5, label, cube);
            octreeLabeling( 6, label, cube);
        }
        if( cube[2] == 1 )
            cube[2] = label;
        if( cube[5] == 1 )
        {
            cube[5] = label;
            octreeLabeling( 4, label, cube);
        }
        if( cube[11] == 1 )
        {
            cube[11] = label;
            octreeLabeling( 6, label, cube);
        }
        if( cube[13] == 1 )
        {
            cube[13] = label;
            octreeLabeling( 4, label, cube);
            octreeLabeling( 6, label, cube);
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==3 )
    {
        if( cube[3] == 1 )
        {
            cube[3] = label;
            octreeLabeling( 1, label, cube);
        }
        if( cube[4] == 1 )
        {
            cube[4] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 2, label, cube);
            octreeLabeling( 4, label, cube);
        }
        if( cube[12] == 1 )
        {
            cube[12] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 5, label, cube);
            octreeLabeling( 7, label, cube);
        }
        if( cube[6] == 1 )
            cube[6] = label;
        if( cube[7] == 1 )
        {
            cube[7] = label;
            octreeLabeling( 4, label, cube);
        }
        if( cube[14] == 1 )
        {
            cube[14] = label;
            octreeLabeling( 7, label, cube);
        }
        if( cube[15] == 1 )
        {
            cube[15] = label;
            octreeLabeling( 4, label, cube);
            octreeLabeling( 7, label, cube);
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==4 )
    {
        if( cube[4] == 1 )
        {
            cube[4] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 2, label, cube);
            octreeLabeling( 3, label, cube);
        }
        if( cube[5] == 1 )
        {
            cube[5] = label;
            octreeLabeling( 2, label, cube);
        }
        if( cube[13] == 1 )
        {
            cube[13] = label;
            octreeLabeling( 2, label, cube);
            octreeLabeling( 6, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[7] == 1 )
        {
            cube[7] = label;
            octreeLabeling( 3, label, cube);
        }
        if( cube[15] == 1 )
        {
            cube[15] = label;
            octreeLabeling( 3, label, cube);
            octreeLabeling( 7, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[8] == 1 )
            cube[8] = label;
        if( cube[16] == 1 )
        {
            cube[16] = label;
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==5 )
    {
        if( cube[9] == 1 )
        {
            cube[9] = label;
            octreeLabeling( 1, label, cube);
        }
        if( cube[10] == 1 )
        {
            cube[10] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 2, label, cube);
            octreeLabeling( 6, label, cube);
        }
        if( cube[12] == 1 )
        {
            cube[12] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 3, label, cube);
            octreeLabeling( 7, label, cube);
        }
        if( cube[17] == 1 )
            cube[17] = label;
        if( cube[18] == 1 )
        {
            cube[18] = label;
            octreeLabeling( 6, label, cube);
        }
        if( cube[20] == 1 )
        {
            cube[20] = label;
            octreeLabeling( 7, label, cube);
        }
        if( cube[21] == 1 )
        {
            cube[21] = label;
            octreeLabeling( 6, label, cube);
            octreeLabeling( 7, label, cube);
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==6 )
    {
        if( cube[10] == 1 )
        {
            cube[10] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 2, label, cube);
            octreeLabeling( 5, label, cube);
        }
        if( cube[11] == 1 )
        {
            cube[11] = label;
            octreeLabeling( 2, label, cube);
        }
        if( cube[13] == 1 )
        {
            cube[13] = label;
            octreeLabeling( 2, label, cube);
            octreeLabeling( 4, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[18] == 1 )
        {
            cube[18] = label;
            octreeLabeling( 5, label, cube);
        }
        if( cube[21] == 1 )
        {
            cube[21] = label;
            octreeLabeling( 5, label, cube);
            octreeLabeling( 7, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[19] == 1 )
            cube[19] = label;
        if( cube[22] == 1 )
        {
            cube[22] = label;
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==7 )
    {
        if( cube[12] == 1 )
        {
            cube[12] = label;
            octreeLabeling( 1, label, cube);
            octreeLabeling( 3, label, cube);
            octreeLabeling( 5, label, cube);
        }
        if( cube[14] == 1 )
        {
            cube[14] = label;
            octreeLabeling( 3, label, cube);
        }
        if( cube[15] == 1 )
        {
            cube[15] = label;
            octreeLabeling( 3, label, cube);
            octreeLabeling( 4, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[20] == 1 )
        {
            cube[20] = label;
            octreeLabeling( 5, label, cube);
        }
        if( cube[21] == 1 )
        {
            cube[21] = label;
            octreeLabeling( 5, label, cube);
            octreeLabeling( 6, label, cube);
            octreeLabeling( 8, label, cube);
        }
        if( cube[23] == 1 )
            cube[23] = label;
        if( cube[24] == 1 )
        {
            cube[24] = label;
            octreeLabeling( 8, label, cube);
        }
    }
    if( octant==8 )
    {
        if( cube[13] == 1 )
        {
            cube[13] = label;
            octreeLabeling( 2, label, cube);
            octreeLabeling( 4, label, cube);
            octreeLabeling( 6, label, cube);
        }
        if( cube[15] == 1 )
        {
            cube[15] = label;
            octreeLabeling( 3, label, cube);
            octreeLabeling( 4, label, cube);
            octreeLabeling( 7, label, cube);
        }
        if( cube[16] == 1 )
        {
            cube[16] = label;
            octreeLabeling( 4, label, cube);
        }
        if( cube[21] == 1 )
        {
            cube[21] = label;
            octreeLabeling( 5, label, cube);
            octreeLabeling( 6, label, cube);
            octreeLabeling( 7, label, cube);
        }
        if( cube[22] == 1 )
        {
            cube[22] = label;
            octreeLabeling( 6, label, cube);
        }
        if( cube[24] == 1 )
        {
            cube[24] = label;
            octreeLabeling( 7, label, cube);
        }
        if( cube[25] == 1 )
            cube[25] = label;
    }
}
int getThinningIterations()
{
    return iterations;
}
void MainWindow::on_btnWrite_clicked()
{
    FILE *ft,*fhdr;
    int i,j,k;
    unsigned short pixel;
    QString outFileName = "",outHdrFileName = "";
    switch(op_type)
    {
    case 1:
        qDebug()<<"Writing binary output image";
        outFileName = imgFileName+"_binary.img";
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim;k++)
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    pixel=binImg[i][j][k];
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"Binary Image written successfully";
        break;
    case 2:qDebug()<<"Writing Distance transform output image";
        ui->lblOuput->setText("Writing DT Image");
        outFileName = "dt_"+imgFileName+".img";
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim;k++)
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    pixel=dtArray[i][j][k];
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"DT Image written successfully";
        break;
    case 3:qDebug()<<"Writing Skeleton output image";
        outFileName = imgFileName+"_skeleton.img";
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim;k++)
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    pixel=skelImg[i][j][k];
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"Skeleton Image written successfully";
        break;
    case 4:
        qDebug()<<"Writing Noise Ouput image";
        outFileName = imgFileName+"_downsampled_noisy_Output.img";
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim/steps;k++)
        {
            for(j=0;j<Ydim/steps;j++)
            {
                for(i=0;i<Xdim/steps;i++)
                {
                    pixel=b[i][j][k];
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"Noisy Image written successfully";
        break;
    case 5:
        qDebug()<<"Writing phantom image";
        ui->lblOuput->setText("Writing phantom Image");
        outFileName = imgFileName+"_phantom.img";
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim;k++)
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    pixel=opImg[i][j][k];
                    if(pixel!=normalIntensity)
                        pixel = 0;
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"Phantom Image written successfully";
        ui->lblOuput->setText("Phantom Image written successfully");
        break;
    case 6:
        qDebug()<<"Writing Seeded image";
        ui->lblOuput->setText("Writing Seeded Image");
        outFileName = imgFileName+"_seeded.img";
        outHdrFileName = imgFileName+"_seeded.hdr";
        fhdr=fopen(outHdrFileName.toStdString().c_str(),"wb");
        if(fhdr==NULL)
        {
            qDebug()<<"Unable to write "+outHdrFileName;
        }
        else
            fwrite(hdr,348,1,fhdr);
        fclose(fhdr);
        ft=fopen(outFileName.toStdString().c_str(),"wb");
        if(ft==NULL)
        {
            printf("\n Unable to open o/p file");
            exit(0);
        }
        for(k=0;k<Zdim;k++)
        {
            for(j=0;j<Ydim;j++)
            {
                for(i=0;i<Xdim;i++)
                {
                    pixel=opImg[i][j][k];
                    fwrite(&pixel,sizeof(unsigned short),1,ft);
                }
            }
        }
        fclose(ft);
        qDebug()<<"Seeded Image written successfully";
        ui->lblOuput->setText("Seeded Image written successfully");
        break;
    default:
        break;
    }
}
void MainWindow::on_btnAddPoint_clicked()
{
    ctrlX[pointCounter] = ui->horizontalSliderX->value();
    ctrlY[pointCounter] = ui->horizontalSliderY->value();
    ctrlZ[pointCounter] = ui->horizontalSliderZ->value();
    dtPoint[pointCounter] = dtArray[ui->horizontalSliderX->value()][ui->horizontalSliderY->value()][ui->horizontalSliderZ->value()];
    QFile file;
    QString fileName = "coordinates.txt";
    file.setFileName(fileName);
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug()<<"Could not create/open file coordinates.txt";
        return;
    }
    QTextStream out(&file);
    out << ctrlX[pointCounter] << " " << ctrlY[pointCounter] <<" "<<ctrlZ[pointCounter]<<" "<<dtPoint[pointCounter]<<endl;
    pointCounter++;
    switch (pointCounter) {
    case 1:
        ui->lblOuput->setText("1st point added");
        ui->btnDrawBezier->setEnabled(false);
        break;
    case 2:
        ui->lblOuput->setText("2nd point added");
        ui->btnDrawBezier->setEnabled(false);
        break;
    case 3:
        ui->lblOuput->setText("3rd point added");
        ui->btnDrawBezier->setEnabled(false);
        break;
    case 4:
        ui->lblOuput->setText("4th point added");
        ui->btnDrawBezier->setEnabled(true);
        break;
    default:
        break;
    }
    if(pointCounter>3)
    {
        pointCounter = 0;
        //ui->btnDrawBezier->setEnabled(true);
    }
    //else
        //ui->btnDrawBezier->setEnabled(false);
}
void MainWindow::on_btnResetPoints_clicked()
{
    for(int i=0;i<4;i++)
    {
        ctrlX[i] = 0;
        ctrlY[i] = 0;
        ctrlZ[i] = 0;
        dtPoint[i] = 0;
    }
    pointCounter = 0;
}
void MainWindow::on_btnDrawBezier_clicked()
{
    numOfCurves=0;
    QDate currentDate;
    QFile pointsFile("coordinates.txt");
    //unsigned int numOfLines=0;
    double t,tFinal,step;
    tFinal=1.0;
    step = 0.01;
    int range = tFinal/step;
    if (!pointsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->lblOuput->setText("Cannot open file");
    }
    else
    {
        QString line;
        QTextStream inpoints(&pointsFile);
        int c=0;
        while(!inpoints.atEnd())
        {
            line = inpoints.readLine();
            std::stringstream stream(line.toStdString());
            for(int q=0;q<4;q++) {
                if(q==0)
                    stream >> ctrlX[c];
                if(q==1)
                    stream >> ctrlY[c];
                if(q==2)
                    stream >> ctrlZ[c];
                if(q==3)
                    stream >> dtPoint[c];
                if(!stream)
                    break;
            }
            c++;
            if(c%4==0)
            {
                int radius, maxRadius = abs(dtPoint[0]/10);
                radius = maxRadius;
                int minRadius = abs(dtPoint[3]/10);
                if(maxRadius==1&&minRadius==1)
                    maxRadius = 2;
                int radiusChangeStep = range/(maxRadius-minRadius);
                int xCor= 0;
                int yCor= 0;
                int zCor= 0;
                int i=0,j=0;
                for(t=0.0;t<=tFinal;t+=step)
                {
                    xCor=(1-t)*(1-t)*(1-t)*ctrlX[0]+3*t*(1-t)*(1-t)*ctrlX[1]+3*t*t*(1-t)*ctrlX[2]+t*t*t*ctrlX[3];
                    yCor=(1-t)*(1-t)*(1-t)*ctrlY[0]+3*t*(1-t)*(1-t)*ctrlY[1]+3*t*t*(1-t)*ctrlY[2]+t*t*t*ctrlY[3];
                    zCor=(1-t)*(1-t)*(1-t)*ctrlZ[0]+3*t*(1-t)*(1-t)*ctrlZ[1]+3*t*t*(1-t)*ctrlZ[2]+t*t*t*ctrlZ[3];
                    i++;
                    j++;
                    /*if(j%zFrequency==0)
                {
                    if(posiDir)
                        zVal++;
                    else
                        zVal--;
                }*/
                    if((i%radiusChangeStep==0)&&(radius>=minRadius))
                    {
                        radius--;
                    }
                    //if(zVal>=4&&zVal<=295)
                    drawsphere(opImg,radius,xCor,yCor,zCor,normalIntensity);
                }
                c=0;
                numOfCurves++;
            }
        }
    }
    pointsFile.close();
    pointsFile.remove();
    op_type = 5;
    qDebug()<<numOfCurves<<" curves generated";
    phantomFlag = 1;
    drawImage(op_type);
    ui->lblOuput->setText("Curve successfully generated");
    ui->btnAddNoise->setEnabled(true);
}
void MainWindow::on_btnReadSeeds_clicked()
{
    for(int i = 0; i<Xdim; i++)
    {
        for(int j = 0; j<Ydim; j++)
        {
            for(int k = 0; k<Zdim; k++)
            {
                opImg[i][j][k] = inputImg[i][j][k];
            }
        }
    }
    //Reading artery and vein seed from file
    bool sepExists = false;
    int *ax,*ay,*az,*vx,*vy,*vz,*sx,*sy,*sz,numOfASeeds=0,numOfVSeeds,numOfSeeds,count=0,c,i,j,k;
    QFile arteryFile(imgFileName+"_arterySeed.txt");
    QFile veinFile(imgFileName+"_veinSeed.txt");
    QFile sepFile(imgFileName+"_sepSeed.txt");
    if(sepFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        sepExists = true;
    }
    if(sepExists)
    {
        c=0;
        numOfSeeds = countSeeds(imgFileName+"_sepSeed.txt");
        sx = (int*)calloc(numOfSeeds,sizeof(int));
        sy = (int*)calloc(numOfSeeds,sizeof(int));
        sz = (int*)calloc(numOfSeeds,sizeof(int));
        sepFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream inSep(&sepFile);
        while (!inSep.atEnd()) {
            QString line = inSep.readLine();
            std::stringstream stream(line.toStdString());
            for(int q=0;q<3;q++) {
                if(q==0)
                    stream >> sx[c];
                if(q==1)
                    stream >> sy[c];
                if(q==2)
                    stream >> sz[c];
                if(!stream)
                    break;
            }
            c++;
        }
        for(int a=0;a<numOfSeeds;a++)
        {
            drawsphere(opImg,7,sx[a],sy[a],sz[a],sepInten);
            qDebug()<<"Marking "<<sx[a]<<","<<sy[a]<<","<<sz[a]<<" as separator";
            for(int n=0;n<6;n++)
            {
                i = sx[a] + sixnpp[n][0];
                j = sy[a] + sixnpp[n][1];
                k = sz[a] + sixnpp[n][2];
                if(i>=0&&i<Xdim&&j>=0&&j<Ydim&&k>=0&&k<Zdim)
                {
                    drawsphere(opImg,7,i,j,k,sepInten);
                    qDebug()<<"Marking "<<i<<","<<j<<","<<k<<" as separator";
                }
            }
        }
    }
    if ((!arteryFile.open(QIODevice::ReadOnly | QIODevice::Text))||(!veinFile.open(QIODevice::ReadOnly | QIODevice::Text)))
    {
        QMessageBox::warning(this, tr("Error"),tr("Unable to open seed files"),QMessageBox::Close);
    }
    else
    {
        numOfASeeds = countSeeds(imgFileName+"_arterySeed.txt");
        ax = (int*)calloc(numOfASeeds,sizeof(int));
        ay = (int*)calloc(numOfASeeds,sizeof(int));
        az = (int*)calloc(numOfASeeds,sizeof(int));
        //arteryFile.open(QIODevice::ReadOnly | QIODevice::Text);
        numOfVSeeds = countSeeds(imgFileName+"_veinSeed.txt");
        vx = (int*)calloc(numOfVSeeds,sizeof(int));
        vy = (int*)calloc(numOfVSeeds,sizeof(int));
        vz = (int*)calloc(numOfVSeeds,sizeof(int));
        //veinFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream inArtery(&arteryFile);
        QTextStream inVein(&veinFile);
        while (!inArtery.atEnd()) {
            QString line = inArtery.readLine();
            std::stringstream stream(line.toStdString());
            for(int q=0;q<3;q++) {
                if(q==0)
                    stream >> ax[count];
                if(q==1)
                    stream >> ay[count];
                if(q==2)
                    stream >> az[count];
                if(!stream)
                    break;
            }
            count++;
        }
        qDebug()<<"Found "<<count<<" artery seeds";
        count=0;
        while (!inVein.atEnd()) {
            QString line = inVein.readLine();
            std::stringstream stream(line.toStdString());
            for(int q=0;q<3;q++) {
                if(q==0)
                    stream >> vx[count];
                if(q==1)
                    stream >> vy[count];
                if(q==2)
                    stream >> vz[count];
                if(!stream)
                    break;
            }
            count++;
        }
        qDebug()<<"Found "<<count<<" vein seeds";
        arteryFile.close();
        veinFile.close();
        sepFile.close();
    }
    for(c=0;c<numOfASeeds;c++)
    {
        qDebug()<<"Marking "<<ax[c]<<","<<ay[c]<<","<<az[c]<<" as artery";
        drawsphere(opImg,7,ax[c],ay[c],az[c],arteryInten);
    }
    for(c=0;c<numOfVSeeds;c++)
    {
        qDebug()<<"Marking "<<vx[c]<<","<<vy[c]<<","<<vz[c]<<" as vein";
        drawsphere(opImg,7,vx[c],vy[c],vz[c],veinInten);
    }
    op_type = 6;
    on_btnWrite_clicked();
}
void MainWindow::on_btnAddNoise_clicked()
{
    //unsigned char pix;
    int i,j,k,p,q,r,s=0,t=0,v=0,sum;
    qDebug()<<"Initializing variables";
    b = new unsigned short**[Xdim/steps];
    for(int i = 0;i<Xdim/steps;i++)
    {
        b[i] = new unsigned short*[Ydim/steps];
        for(int j = 0;j<Ydim/steps;j++)
        {
            b[i][j] = new unsigned short[Zdim/steps];
        }
    }
    noise = new short**[Xdim/steps];
    for(int i = 0;i<Xdim/steps;i++)
    {
        noise[i] = new short*[Ydim/steps];
        for(int j = 0;j<Ydim/steps;j++)
        {
            noise[i][j] = new short[Zdim/steps];
        }
    }
    checknoise = new short**[Xdim/steps];
    for(int i = 0;i<Xdim/steps;i++)
    {
        checknoise[i] = new short*[Ydim/steps];
        for(int j = 0;j<Ydim/steps;j++)
        {
            checknoise[i][j] = new short[Zdim/steps];
        }
    }
    avgnoise = new short**[Xdim/steps];
    for(int i = 0;i<Xdim/steps;i++)
    {
        avgnoise[i] = new short*[Ydim/steps];
        for(int j = 0;j<Ydim/steps;j++)
        {
            avgnoise[i][j] = new short[Zdim/steps];
        }
    }
    qDebug()<<"Variables initialized";
    for(k=0;k<Zdim;k+=steps)
    {
        t=0;
        for(j=0;j<Ydim;j+=steps)
        {
            s=0;
            for(i=0;i<Xdim;i+=steps)
            {
                sum=0;
                for(r=k-ws;r<=k+ws;r++)
                {
                    for(q=j-ws;q<=j+ws;q++)
                    {
                        for(p=i-ws;p<=i+ws;p++)
                        {
                            if(p>=0&&p<Xdim&&q>=0&&q<Ydim&&r>=0&&r<Zdim)
                            {
                                if(phantomFlag!=1)
                                    sum=sum+inputImg[p][q][r];
                                else
                                    sum=sum+opImg[p][q][r];
                            }
                        }
                    }
                }
                avg=sum/(double)((2*ws+1)*(2*ws+1)*(2*ws+1));
                avg=round(avg);
                avg1=(unsigned short)avg;
                if(s>=0&&s<(Xdim/steps)&&t>=0&&t<(Ydim/steps)&&v>=0&&(Zdim/steps))
                    b[s][t][v]=avg1;
                s+=1;
            }
            t+=1;
        }
        v+=1;
    }
    for(k=0;k<(Zdim/steps);k++)
    {
        for(j=0;j<(Ydim/steps);j++)
        {
            for(i=0;i<(Xdim/steps);i++)
            {
                if(b[i][j][k])
                    pixcount++;
            }
        }
    }
    qDebug()<<"Number of object pixel in image::"<<pixcount;
    qDebug()<<"Adding Noise";
    addnoise();
    qDebug()<<"Noise added\nRandomizing noise pixels";
    selectnoisevox();
    op_type = 4;
    qDebug()<<"Displaying Noisy image";

    ui->lblXY->setMinimumHeight(Ydim);
    ui->lblXY->setMinimumWidth(Xdim);

    ui->lblYZ->setMinimumHeight(Zdim);
    ui->lblYZ->setMinimumWidth(Ydim);

    ui->lblXZ->setMinimumHeight(Xdim);
    ui->lblXZ->setMinimumWidth(Zdim);

    CurrentX = Xdim/2;
    CurrentY = Ydim/2;
    CurrentZ = Zdim/2;

    ui->horizontalSliderX->setValue(Xdim/(2*steps));
    ui->horizontalSliderY->setValue(Ydim/(2*steps));
    ui->horizontalSliderZ->setValue(Zdim/(2*steps));
    ui->horizontalSliderX->setRange(0,(Xdim/steps)-1);
    ui->horizontalSliderY->setRange(0,(Ydim/steps)-1);
    ui->horizontalSliderZ->setRange(0,(Zdim/steps)-1);
    ui->spinBoxX->setRange(0,(Xdim/steps)-1);
    ui->spinBoxY->setRange(0,(Ydim/steps)-1);
    ui->spinBoxZ->setRange(0,(Zdim/steps)-1);
    drawImage(op_type);
    noiseFlag = 1;
    ui->lblOuput->setText("Noisy image generation complete");
}
