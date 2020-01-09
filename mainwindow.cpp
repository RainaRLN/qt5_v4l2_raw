#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capture.h"
#include "convert.h"
#include "config.h"
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

    d = new HoughParm(WIDTH, HEIGHT, this);
    d->setModal(false);

    grbg = static_cast<unsigned char *>(malloc(WIDTH * HEIGHT * sizeof(char)));
    if(nullptr == grbg)
    {
        perror("pchangevideodata error");
    }
    bgr = static_cast<unsigned char *>(malloc(WIDTH * HEIGHT * 3 * sizeof(char)));
    if(nullptr == bgr)
    {
        perror("pchangevideodata error");
    }

    timer->setInterval(1);

    open_device();  // 打开摄像头设备
    set_fmt();  // 设置当前格式
    init_reqbuf();  // 申请缓冲区
    stream_on();
    this->show_offset = 0;
    ui->image->setScaledContents(true);
    qs = ui->image->rect().size();
    ui->centroid->setText(QString().sprintf("(%d, %d)", WIDTH/2, HEIGHT/2));

    connect(timer, SIGNAL(timeout()), this, SLOT(showImage()));
    i2c_fd = open_i2cdev(I2C_DEV);
    if (i2c_fd < 0) {
        show_log("Couldn't open i2c device!");
        ui->i2c->setEnabled(false);
        ui->ois->setEnabled(false);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_start_clicked()
{
    timer->start();
    show_log("Stream on");
}

void MainWindow::on_stop_clicked()
{
    timer->stop();
    show_log("Stream off");
}

void MainWindow::on_capture_clicked()
{
    QString path = dir + "/" + ui->filename->text();
    SaveBmp(path.toUtf8().data(), bgr, WIDTH, HEIGHT);
    show_log("Captured one picture: " + ui->filename->text());
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
    if (!QString(choose).isEmpty())
        dir = choose;
    show_log("Save Path: " + dir + "/" + ui->filename->text());
    timer->start();

}

void MainWindow::showImage()
{
    get_frame(reinterpret_cast<void **>(&bggr), &uDataLength);
    GRBG2BGR(WIDTH, HEIGHT, bggr, bgr);
    if (this->show_offset) {
        d->handle(bgr, 1);
        ui->offset->setText(QString().sprintf("(%d, %d)", d->x1, d->y1));
    }
    tempImage = new QImage(static_cast<const uchar*>(bgr), WIDTH ,HEIGHT, WIDTH*3, QImage::Format_RGB888);

    ui->image->setPixmap(QPixmap::fromImage((*tempImage)));
    //ui->image->setPixmap(QPixmap::fromImage((*tempImage).mirrored(false, true)).scaled(qs));
}

void MainWindow::on_exit_clicked()
{
    stream_off();
    this->close();
}

void MainWindow::on_i2c_clicked()
{
    OV_DATA ov_data;
    FILE *cfg;
    static QString i2cFile;
    timer->stop();
    if (QString(i2cFile).isEmpty()) {
        i2cFile = QFileDialog::getOpenFileName(
                    this,
                    tr("Open IIC File"),
                    "../configFiles/",
                    "TXT Files(*.txt)"
                    );
    }

    if (!QString(i2cFile).isEmpty()) {
        show_log("i2c FilePath: " + i2cFile);

        if ((cfg=fopen(i2cFile.toUtf8().data(), "r")) == nullptr) {  // 判断文件是否正常打开
            show_log("Config File Open Error!");
            goto iic_cfg_exit;  // 打开失败就退出
        }
        show_log("Config File Open Successfully!");
        int row = 0;
        while (!feof(cfg)) {  // 文件没到结尾
            fscanf(cfg, "%2hhx%2hhx %hhx", &(ov_data.package.reg_addrH), &(ov_data.package.reg_addrL), &(ov_data.package.value));  // 十六进制方式读取一个数
            // TODO 跳过空行
            if (ioctl(i2c_fd, I2C_SLAVE_FORCE, OV_ADDR) < 0) {
                qDebug("I2C_SLAVE_FORCE failed");
                goto iic_cfg_exit;  // 打开失败就退出
            }
            i2c_transfer(i2c_fd, OV_ADDR, 0, ov_data.i2c_buf, 3);
            ui->regTable->insertRow(row);
            ui->regTable->setItem(row, 0, new QTableWidgetItem(QString().sprintf("0x%02hhx%02hhx", ov_data.package.reg_addrH, ov_data.package.reg_addrL)));
            ui->regTable->setItem(row, 1, new QTableWidgetItem(QString().sprintf("0x%02hhx", ov_data.package.value)));
            ++row;
        }
        fclose(cfg);  //操作完毕关闭文件。
        while (get_frame(reinterpret_cast<void **>(&bggr), &uDataLength) > 0);
        show_log("Write Config Successfully!");
    } else {
        show_log("Couldn't get IIC Config File Path!");
    }

iic_cfg_exit:
    timer->start();
    return;

}

void MainWindow::on_ois_clicked()
{
    static bool ois_on = false;
    ROHM_DATA ois_switch = {{0}};
    ROHM_DATA ois_check = {{0}};
    static QString prog_file;
    static QString coef_file;
    timer->stop();

    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, ROHM_ADDR) < 0) {
        qDebug("I2C_SLAVE_FORCE failed");
        goto ois_exit;  // 打开失败就退出
    }

    if (QString(prog_file).isEmpty()){
        prog_file = QFileDialog::getOpenFileName(
                    this,
                    tr("Open OIS Program File"),
                    "../configFiles/",
                    "BIN Files(*.bin)"
                    );
    }
    if (QString(coef_file).isEmpty()){
        coef_file = QFileDialog::getOpenFileName(
                    this,
                    tr("Open OIS Coefficient File"),
                    "../configFiles/",
                    "MEM Files(*.mem)"
                    );
    }

    if (QString(prog_file).isEmpty() || QString(coef_file).isEmpty()) {
        show_log("Couldn't get Program or Coefficient File Path!");
        goto ois_exit;
    }

    ois_switch.package.opCode = _OP_SpecialCMD;
    if (ois_on) {  // OIS OFF
        ois_switch.package.data[0] = 0x00;
        i2c_transfer(i2c_fd, ROHM_ADDR, 0, ois_switch.i2c_buf, 2);
        show_log("OIS OFF!");
        ui->ois->setText("OIS ON");
        ois_on = false;
        goto ois_exit;
    }

    // Check
    ois_check.msg.opCode = 0x82;
    ois_check.msg.addr = 0x20;
    ois_check.msg.data[0] = 0xcd;
    ois_check.msg.data[1] = 0xef;
    i2c_transfer(i2c_fd, ROHM_ADDR, 0, ois_check.i2c_buf, 4);

    ois_check.msg.addr = 0x00;
    i2c_transfer(i2c_fd, ROHM_ADDR, 1, ois_check.i2c_buf, 2, ois_check.msg.data, 2);
    show_log(QString().sprintf("OpCode = 0x%02hhx Addr = 0x%02hhx Data = 0x%02hhx%02hhx",
                               ois_check.msg.opCode, ois_check.msg.addr,
                               ois_check.msg.data[0], ois_check.msg.data[1]));
    ois_check.msg.addr = 0x20;
    i2c_transfer(i2c_fd, ROHM_ADDR, 1, ois_check.i2c_buf, 2, ois_check.msg.data, 2);
    show_log(QString().sprintf("OpCode = 0x%02hhx Addr = 0x%02hhx Data = 0x%02hhx%02hhx",
                               ois_check.msg.opCode, ois_check.msg.addr,
                               ois_check.msg.data[0], ois_check.msg.data[1]));
    // Download Program
    if (ois_file_download(i2c_fd, prog_file.toUtf8().data(), _OP_FIRM_DWNLD) < 0) {
        show_log("Download Program Failed!");
        goto ois_exit;
    }


    // Download Coefficient
    if (ois_file_download(i2c_fd, coef_file.toUtf8().data(), _OP_COEF_DWNLD) < 0) {
        show_log("Download Coefficient Failed!");
        goto ois_exit;
    }

    // OIS ON
    ois_switch.package.data[0] = 0x01;
    i2c_transfer(i2c_fd, ROHM_ADDR, 0, ois_switch.i2c_buf, 2);
    show_log("OIS ON!");
    ui->ois->setText("OIS OFF");
    ois_on = true;

ois_exit:
    timer->start();
    return;
}

void MainWindow::show_log(QString log)
{
    ui->showLog->append("[" + QDateTime::currentDateTime().toString() + "] " + log);
}

void MainWindow::on_get_centroid_clicked()
{
    get_frame(reinterpret_cast<void **>(&bggr), &uDataLength);
    GRBG2BGR(WIDTH, HEIGHT, bggr, bgr);
    d->show();
    d->handle(bgr, 2);
    ui->centroid->setText(QString().sprintf("(%d, %d)", d->x, d->y));
    ui->offset->setText(QString().sprintf("(%d, %d)", d->x1, d->y1));
}

void MainWindow::on_show_offset_clicked()
{
    this->show_offset = (++(this->show_offset) % 2);
}
