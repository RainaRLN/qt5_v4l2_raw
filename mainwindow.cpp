#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capture.h"
#include "convert.h"
#include <QPixmap>
#include <QImage>
#include <QSize>
#include <QFileDialog>
#include <QDesktopWidget>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width()-this->width())/2, (desktop->height()-this->height())/2);

    grbg = (unsigned char *)malloc(WIDTH * HEIGHT * sizeof(char));
    if(nullptr == grbg)
    {
        perror("pchangevideodata error");
    }
    bgr = (unsigned char *)malloc(WIDTH * HEIGHT * 3 * sizeof(char));
    if(nullptr == bgr)
    {
        perror("pchangevideodata error");
    }

    timer->setInterval(1);

    open_device();  // 打开摄像头设备
    //get_capabilities();
    //enum_fmt();
    //try_fmt();
    set_fmt();  // 设置当前格式
    //get_fmt();
    init_reqbuf();  // 申请缓冲区
    stream_on();

    ui->image->setScaledContents(true);
    qs = ui->image->rect().size();

    connect(timer, SIGNAL(timeout()), this, SLOT(showImage()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_start_clicked()
{
    ui->choosePath->setEnabled(false);
    timer->start();
    ui->showLog->setText("Stream on");
}

void MainWindow::on_stop_clicked()
{
    timer->stop();
    ui->showLog->setText("Stream off");
    ui->choosePath->setEnabled(true);
}

void MainWindow::on_capture_clicked()
{
    QString path = dir + "/" + ui->filename->text();
    qDebug() << path;
    SaveBmp1(path.toUtf8().data(), bgr, WIDTH, HEIGHT);
    ui->showLog->setText("Captured one picture: " + ui->filename->text());
}

void MainWindow::on_choosePath_clicked()
{
    QString choose;
    choose = QFileDialog::getExistingDirectory(
                this,
                tr("Open Directory"),
                "../",
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks
                );
    if(!QString(choose).isEmpty())
        dir = choose;
    ui->showLog->setText("Save Path: " + dir + "/" + ui->filename->text());

}

void MainWindow::showImage()
{
    unsigned short a = 2;
    while(a--)
    {
        get_frame((void **)(&bggr), &uDataLength);
    }
    GRBG2BGR(WIDTH, HEIGHT, bggr, bgr);
    rgb_adjust(WIDTH, HEIGHT, bgr, 0.93);
    QImage *tempImage = new QImage((const uchar*)(bgr), WIDTH ,HEIGHT, WIDTH*3, QImage::Format_RGB888);
    ui->image->setPixmap(QPixmap::fromImage((*tempImage).mirrored(false, true)).scaled(qs));
}

void MainWindow::on_exit_clicked()
{
    stream_off();
    this->close();
}
