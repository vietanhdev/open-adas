#ifndef FOUR_POINT_SELECT_PAGE_H
#define FOUR_POINT_SELECT_PAGE_H

#include <mutex>
#include <string>
#include <vector>
#include <QImage>
#include <QWizardPage>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>
#include "ui_four_point_select_page.h"
#include "car_status.h"
// #include "../point.h"

class FourPointSelectPage : public QWizardPage {

    Q_OBJECT

   private:
    Ui::FourPointSelectPage *ui;
    std::shared_ptr<CarStatus> car_status;

    const std::vector<std::string> prefix = {"tl_", "tr_", "br_", "bl_"};

    cv::Mat image;
    std::mutex image_mutex;

    // Top-left, top-right, bottom-right, bottom-left
    std::vector<cv::Point2f> points;
    int current_point_id;
    std::string current_point_title;
    std::mutex points_mutex;

    float tl_x;

   public:
    FourPointSelectPage(QWidget*, std::shared_ptr<CarStatus> car_status);
    cv::Mat getImage();
    void updateVisualization();
    void pointToScrollbar();

   private:
    void selectPoint(std::string title, int point_id);
    QDoubleSpinBox *getDummyDoubleSpinBox();
    void getLinePointinImageBorder(const cv::Point &p1_in,
                                                    const cv::Point &p2_in,
                                                    cv::Point &p1_out,
                                                    cv::Point &p2_out, int rows,
                                                    int cols);

   private slots:
    void getNewImageFromCam();
    void updateY(int value);
    void updateX(int value);
    void selectTopLeftPoint();
    void selectTopRightPoint();
    void selectBottomRightPoint();
    void selectBottomLeftPoint();


};

#endif