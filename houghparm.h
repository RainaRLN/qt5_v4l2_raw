#ifndef HOUGHPARM_H
#define HOUGHPARM_H
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <QDialog>
#include <QSize>
#include <QKeyEvent>

namespace Ui {
class HoughParm;
}
using namespace cv;

class HoughParm : public QDialog
{
    Q_OBJECT

public:
    explicit HoughParm(unsigned int w, unsigned int h, QWidget *parent = nullptr);
    ~HoughParm();

    double bin_thresh = 20;

    double canny_thresh1 = 0;
    double canny_thresh2 = 10;
    int    canny_Ssize = 7;  // Soble Size

    double hough_thresh = 150;
    double hough_minlen = 150;
    double hough_maxgap = 80;

    double shi_quality = 0.05;
    double shi_mindist = 35;
    int    shi_Bsize = 3;  // Block Size
    bool   shi_usehough = true;
    double shi_houghk = 0.04;

    int x;
    int y;

    int x1;
    int y1;

    void handle(unsigned char *bgr, int mod);

private slots:
    void on_exit_clicked();

    void on_bin_thresh_editingFinished();

    void on_canny_thresh1_editingFinished();

    void on_canny_thresh2_editingFinished();

    void on_canny_Ssize_editingFinished();

    void on_hough_thresh_editingFinished();

    void on_hough_minlen_editingFinished();

    void on_hough_maxgap_editingFinished();

    void on_hough_centroid_clicked();

    void on_shi_quality_editingFinished();

    void on_shi_mindist_editingFinished();

    void on_shi_Bsize_editingFinished();

    void on_shi_useHough_clicked();

    void on_shi_hough_k_editingFinished();

    void on_shi_centroid_clicked();

private:
    Ui::HoughParm *ui;
    QSize qs;
    unsigned int width;
    unsigned int height;
    int mode = 2;

    Mat origin_chart;
    Mat gray_chart;
    Mat binary_chart;
    Mat canny_chart;
    Mat hough_chart;
    Mat shi_chart;
    Mat corner_chart;

    void init();
    void show_binaryPic();
    void show_cannyPic();
    void show_houghlinePic();
    void show_cornerPic();

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // HOUGHPARM_H
