#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include "houghparm.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void show_log(QString log);

private slots:
    void on_start_clicked();

    void on_stop_clicked();

    void on_capture_clicked();

    void on_choosePath_clicked();

    void showImage();

    void on_exit_clicked();

    void on_i2c_clicked();

    void on_ois_clicked();

    void on_get_centroid_clicked();

    void on_show_offset_clicked();

private:
    Ui::MainWindow *ui;
    QSize qs;
    QTimer *timer = new QTimer();
    QString dir = "/home/firefly";
    int i2c_fd;
    QImage *tempImage;
    HoughParm *d;
    int show_offset;
};

#endif // MAINWINDOW_H
