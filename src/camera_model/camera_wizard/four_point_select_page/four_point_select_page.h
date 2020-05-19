#ifndef FOUR_POINT_SELECT_PAGE_H
#define FOUR_POINT_SELECT_PAGE_H

#include <mutex>
#include <QFileDialog>
#include <QWizardPage>
#include <opencv2/opencv.hpp>
#include "ui_four_point_select_page.h"
#include "car_status.h"

class FourPointSelectPage : public QWizardPage {
   private:
    Ui::FourPointSelectPage *ui;
    CarStatus *car_status;

    cv::Mat image;
    std::mutex image_mutex;

   public:
    FourPointSelectPage(QWidget*, CarStatus *car_status);
    void getNewImageFromCam();
    void setImage(const cv::Mat &image);
    cv::Mat getImage();

};

#endif