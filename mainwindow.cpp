#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capture.h"
#include "convert.h"
#include "reg_config.h"
#include <QPixmap>
#include <QImage>
#include <QDateTime>
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

    timer->setInterval(2);

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
    timer->start();
    ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Stream on");
}

void MainWindow::on_stop_clicked()
{
    timer->stop();
    ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Stream off");
}

void MainWindow::on_capture_clicked()
{
    QString path = dir + "/" + ui->filename->text();
    SaveBmp1(path.toUtf8().data(), bgr, WIDTH, HEIGHT);
    ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Captured one picture: " + ui->filename->text());
}

void MainWindow::on_choosePath_clicked()
{
    timer->stop();
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
    ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Save Path: " + dir + "/" + ui->filename->text());
    timer->start();

}

void MainWindow::showImage()
{
    unsigned short a = 1;
    while(a--)
    {
        get_frame((void **)(&bggr), &uDataLength);
    }
    GRBG2BGR(WIDTH, HEIGHT, bggr, bgr);
    //rgb_adjust(WIDTH, HEIGHT, bgr, 0.93);
    QImage *tempImage = new QImage((const uchar*)(bgr), WIDTH ,HEIGHT, WIDTH*3, QImage::Format_RGB888);
    ui->image->setPixmap(QPixmap::fromImage((*tempImage)).scaled(qs));
    //ui->image->setPixmap(QPixmap::fromImage((*tempImage).mirrored(false, true)).scaled(qs));
}

void MainWindow::on_exit_clicked()
{
    stream_off();
    this->close();
}

void MainWindow::on_i2c_clicked()
{
    int res;
    int i2c_fd;
    int reg, value;
    FILE *cfg;
    char i2c_buf[50];
    QString i2cFile;
    timer->stop();
    i2cFile = QFileDialog::getOpenFileName(
                this,
                tr("Open IIC File"),
                "../",
                "TXT Files(*.txt)"
                );
    if(!QString(i2cFile).isEmpty()) {
        ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "i2c FilePath: " + i2cFile);
        i2c_fd = open(I2C_DEV, O_RDWR);
        if(i2c_fd < 0){
               ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "i2c device open failed");
               return;
       }
        res = ioctl(i2c_fd, I2C_TENBIT, 0);  // not 10bit
        if(res< 0){
                ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "I2C_TENBIT failed");
                return;
        }
        res = ioctl(i2c_fd, I2C_SLAVE_FORCE, CHIP_ADDR);  // 设置I2C从设备地址[6:0]
        if(res< 0){
                ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "I2C_SLAVE failed");
                return;
        }
        if ((cfg=fopen(i2cFile.toUtf8().data(), "r")) == NULL) //判断文件是否正常打开
        {
            ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Config File Open Error!");
            return;               //打开失败就退出
        }
        int row = 0;
        while(!feof(cfg))            //文件没到结尾
        {
            fscanf(cfg, "%x %x", &reg, &value);	     //十六进制方式读取一个数
            i2c_buf[0] = value;
            res = iic_write(i2c_fd, i2c_buf, reg, 1);
            ui->regTable->insertRow(row);
            ui->regTable->setItem(row, 0, new QTableWidgetItem(QString::number(reg).sprintf("0x%04x", reg)));
            ui->regTable->setItem(row, 1, new QTableWidgetItem(QString::number(value).sprintf("0x%02x", value)));
            ++row;
        }
        fclose(cfg);               //操作完毕关闭文件。
        while(get_frame((void **)(&bggr), &uDataLength) > 0);
        ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "Write Config Successfully!");
    }
    timer->start();

}

void MainWindow::on_ois_clicked()
{
    static bool ois_on = false;
    int res;
    int i2c_fd;
    char i2c_buf[50];
    i2c_fd = open(I2C_DEV, O_RDWR);
    if(i2c_fd < 0){
           ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "i2c device open failed");
           return;
   }
    res = ioctl(i2c_fd, I2C_TENBIT, 0);  // not 10bit
    if(res< 0){
            ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "I2C_TENBIT failed");
            return;
    }
    res = ioctl(i2c_fd, I2C_SLAVE_FORCE, 0x19);  // 设置I2C从设备地址[6:0]
    if(res< 0){
            ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "I2C_SLAVE failed");
            return;
    }
    i2c_buf[0] = 0x00;i2c_buf[1] = 0x00;i2c_buf[2] = 0x00;i2c_buf[3] = 0x00;i2c_buf[4] = 0x00;i2c_buf[5] = 0x00;i2c_buf[6] = 0x00;
    if (ois_on) {
        res = iic_write(i2c_fd, i2c_buf, 0x0100, 7);
        ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "OIS OFF!");
        ui->ois->setText("OIS ON");
        ois_on = false;
    } else {
        res = iic_write(i2c_fd, i2c_buf, 0x0105, 7);
        ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + "OIS ON!");
        ui->ois->setText("OIS OFF");
        ois_on = true;
    }
}
