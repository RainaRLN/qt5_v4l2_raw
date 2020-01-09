#include "houghparm.h"
#include "ui_houghparm.h"
#include <iostream>
#include <QImage>
#include <QDebug>

using namespace std;

void HoughParm::init()
{
    ui->bin_thresh->setValue(bin_thresh);

    ui->canny_thresh1->setValue(canny_thresh1);
    ui->canny_thresh2->setValue(canny_thresh2);
    ui->canny_Ssize->setValue(canny_Ssize);

    ui->hough_thresh->setValue(hough_thresh);
    ui->hough_minlen->setValue(hough_minlen);
    ui->hough_maxgap->setValue(hough_maxgap);
    ui->hough_centroid->setChecked(false);
    ui->shi_useHough->setChecked(false);
    ui->shi_centroid->setChecked(false);
}

HoughParm::HoughParm(unsigned int w, unsigned int h, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HoughParm)
{
    ui->setupUi(this);
    this->width = w;
    this->height = h;
    this->x = this->width / 2;
    this->y = this->height / 2;
    this->x1 = this->width / 2;
    this->y1 = this->height / 2;
    ui->gray_pic->setScaledContents(true);
    ui->bin_pic->setScaledContents(true);
    ui->canny_pic->setScaledContents(true);
    ui->line_pic->setScaledContents(true);
    ui->corner_pic->setScaledContents(true);
    qs = ui->gray_pic->rect().size();
}

HoughParm::~HoughParm()
{
    delete ui;
}

void HoughParm::handle(unsigned char *bgr, int mod=2)
{
    init();
    this->mode = mod;
    origin_chart = Mat(height, width, CV_8UC3, bgr).clone();

    Mat bf_chart;
    bilateralFilter(origin_chart, bf_chart, 5, 10, 10);
    Mat enhance_chart;
    Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0 , -1, 0);
    filter2D(bf_chart, enhance_chart, CV_8UC3, kernel);

    cvtColor(enhance_chart, gray_chart, COLOR_BGR2GRAY);

    QImage *gray = new QImage(static_cast<const uchar*>(gray_chart.data), width ,height, width, QImage::Format_Grayscale8);
    ui->gray_pic->setPixmap(QPixmap::fromImage((*gray)).scaled(qs));

    show_binaryPic();
    if (mode == 1)
        memcpy(bgr, corner_chart.data, width*height*3);
}

void HoughParm::on_exit_clicked()
{
    this->close();
}

void HoughParm::on_bin_thresh_editingFinished()
{
    show_binaryPic();
}

void HoughParm::show_binaryPic()
{
    bin_thresh = ui->bin_thresh->value();
    threshold(gray_chart, binary_chart, bin_thresh, 255, THRESH_BINARY);
    QImage *binary = new QImage(static_cast<const uchar*>(binary_chart.data), width ,height, width, QImage::Format_Grayscale8);
    ui->bin_pic->setPixmap(QPixmap::fromImage((*binary)));
    show_cannyPic();
    show_cornerPic();
}

void HoughParm::show_cannyPic()
{
    if (mode == 1) {
        ui->canny_pic->setText("canny picture");
        ui->line_pic->setText("get line");
        return;
    }
    canny_thresh1 = ui->canny_thresh1->value();
    canny_thresh2 = ui->canny_thresh2->value();
    canny_Ssize = ui->canny_Ssize->value();
    if (canny_Ssize % 2 == 0)
        canny_Ssize = 3;
    Canny(binary_chart, canny_chart, canny_thresh1, canny_thresh2, canny_Ssize);
    QImage *canny = new QImage(static_cast<const uchar*>(canny_chart.data), width ,height, width, QImage::Format_Grayscale8);
    ui->canny_pic->setPixmap(QPixmap::fromImage((*canny)));
    show_houghlinePic();
}

void HoughParm::show_houghlinePic()
{
    hough_thresh = ui->hough_thresh->value();
    hough_minlen = ui->hough_minlen->value();
    hough_maxgap = ui->hough_maxgap->value();
    vector<Vec4i> lines;
    HoughLinesP(canny_chart, lines, 1, CV_PI/180, hough_thresh, hough_minlen, hough_maxgap);

    Mat hough_chart;
    hough_chart = origin_chart.clone();

    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i ln = lines[i];
        line(hough_chart, Point(ln[0], ln[1]), Point(ln[2], ln[3]), Scalar(255, 0, 0), 2, LINE_AA);
    }

    char lsize[20];
    sprintf(lsize, "%lu lines", lines.size());
    putText(hough_chart, lsize, Point(40, 180), FONT_HERSHEY_DUPLEX, 5, Scalar(0, 255, 0), 3);

    if (ui->hough_centroid->isChecked() && lines.size() == 4) {
        Point2f crosspoint[4];
        int p = 0;
        for (size_t i = 0; i < lines.size(); i++) {
            for (size_t j = i+1; j < lines.size(); j++) {
                double ka, kb;
                ka = (double)(lines[i][3] - lines[i][1]) / (double)(lines[i][2] - lines[i][0]);
                kb = (double)(lines[j][3] - lines[j][1]) / (double)(lines[j][2] - lines[j][0]);

                if ((-1 < abs(ka - kb) && abs(ka - kb) < 0.5) || ka == kb) {
                    continue;
                }

                if (ka+1 == ka-1) {
                    crosspoint[p].x = lines[i][0];
                    crosspoint[p].y = lines[j][1];
                } else if (kb+1 == kb-1) {
                    crosspoint[p].x = lines[j][0];
                    crosspoint[p].y = lines[i][1];
                } else {
                    crosspoint[p].x = (ka*lines[i][0] -lines[i][1] - kb*lines[j][0] + lines[j][1]) / (ka - kb);
                    crosspoint[p].y = (ka*kb*(lines[i][0] - lines[j][0]) + ka*lines[j][1] - kb*lines[i][1]) / (ka - kb);
                }
                circle(hough_chart, crosspoint[p], 8, Scalar(255, 0, 255), -1);
                p++;
            }
        }

        float x_sum = 0;
        float y_sum = 0;
        for (int i = 0; i < 4; i++) {
            x_sum += crosspoint[i].x;
            y_sum += crosspoint[i].y;
        }
        x = x_sum / 4;
        y = y_sum / 4;
        circle(hough_chart, Point(x, y), 8, Scalar(255, 0, 255), -1);
        char x_y[13];
        sprintf(x_y, "(%4d, %4d)", x, y);
        putText(hough_chart, x_y, Point(x+40, y-40), FONT_HERSHEY_DUPLEX, 2, Scalar(0, 255, 0), 2);
    }

    QImage *houghline = new QImage(static_cast<const uchar*>(hough_chart.data), width ,height, width*3, QImage::Format_RGB888);
    ui->line_pic->setPixmap(QPixmap::fromImage((*houghline)));
}

void HoughParm::show_cornerPic()
{
    if (mode == 0) {
        ui->corner_pic->setText("get corners");
        return;
    }

    vector<Point2f> corners;
    int max_corners = 10;
    shi_quality = ui->shi_quality->value();
    shi_mindist = ui->shi_mindist->value();
    shi_Bsize = ui->shi_Bsize->value();
    shi_usehough = ui->shi_useHough->isChecked();
    shi_houghk = ui->shi_hough_k->value();

    Mat roi = Mat::zeros(height, width, CV_8UC1);
    rectangle(roi, Rect(1088, 816, 1088, 816), Scalar(255, 255, 255), -1);

    goodFeaturesToTrack(binary_chart, corners, max_corners, shi_quality,
                            shi_mindist, roi, shi_Bsize, shi_usehough, shi_houghk);

    corner_chart = origin_chart.clone();

    for (size_t i = 0; i < corners.size(); i++) {
        circle(corner_chart, corners[i], 8, Scalar(255, 0, 255), -1);
    }

    if (ui->shi_centroid->isChecked()  && corners.size() == 4) {
        float x_sum = 0;
        float y_sum = 0;
        for (int i = 0; i < 4; i++) {
            x_sum += corners[i].x;
            y_sum += corners[i].y;
        }
        x1 = x_sum / 4;
        y1 = y_sum / 4;
        circle(corner_chart, Point(x, y), 8, Scalar(255, 0, 255), -1);
        char x_y[13];
        sprintf(x_y, "(%4d, %4d)", x, y);
        putText(corner_chart, x_y, Point(x+40, y-40), FONT_HERSHEY_DUPLEX, 2, Scalar(0, 255, 0), 2);
    }

    QImage *corner = new QImage(static_cast<const uchar*>(corner_chart.data), width ,height, width*3, QImage::Format_RGB888);
    ui->corner_pic->setPixmap(QPixmap::fromImage((*corner)));
}

void HoughParm::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}

void HoughParm::on_canny_thresh1_editingFinished()
{
    show_cannyPic();
}

void HoughParm::on_canny_thresh2_editingFinished()
{
    show_cannyPic();
}

void HoughParm::on_canny_Ssize_editingFinished()
{
    show_cannyPic();
}

void HoughParm::on_hough_thresh_editingFinished()
{
    show_houghlinePic();
}

void HoughParm::on_hough_minlen_editingFinished()
{
    show_houghlinePic();
}

void HoughParm::on_hough_maxgap_editingFinished()
{
    show_houghlinePic();
}

void HoughParm::on_hough_centroid_clicked()
{
    show_houghlinePic();
}

void HoughParm::on_shi_quality_editingFinished()
{
    show_cornerPic();
}

void HoughParm::on_shi_mindist_editingFinished()
{
    show_cornerPic();
}

void HoughParm::on_shi_Bsize_editingFinished()
{
    show_cornerPic();
}

void HoughParm::on_shi_useHough_clicked()
{
    show_cornerPic();
}

void HoughParm::on_shi_hough_k_editingFinished()
{
    show_cornerPic();
}

void HoughParm::on_shi_centroid_clicked()
{
    show_cornerPic();
}
