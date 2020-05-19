#ifndef FOUR_POINT_SELECT_PAGE_H
#define FOUR_POINT_SELECT_PAGE_H

#include <mutex>
#include <string>
#include <vector>
#include <QImage>
#include <QWizardPage>
#include <opencv2/opencv.hpp>
#include "ui_four_point_select_page.h"
#include "car_status.h"
#include "../point.h"

class FourPointSelectPage : public QWizardPage {

    Q_OBJECT

   private:
    Ui::FourPointSelectPage *ui;
    CarStatus *car_status;

    cv::Mat image;
    std::mutex image_mutex;

    // Top-left, top-right, bottom-right, bottom-left
    std::vector<Point> points;
    int current_point_id;
    std::string current_point_title;
    std::mutex points_mutex;

   public:
    FourPointSelectPage(QWidget*, CarStatus *car_status);
    void setImage(const cv::Mat &image);
    cv::Mat getImage();
    void updateVisualization();
    void pointToScrollbar();

   private:
    void selectPoint(std::string title, int point_id);

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