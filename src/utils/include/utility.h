#if !defined(UTILITY_H)
#define UTILITY_H

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <QImage>

namespace ml_cam {

    void setLabel(cv::Mat& im, const std::string label, const cv::Point & origin);
    std::string getHomePath();

    QImage Mat2QImage(cv::Mat const& src);
    cv::Mat QImage2Mat(QImage const& src);
    cv::Mat resizeByMaxSize(const cv::Mat &img, int max_size);

}

#endif // UTILITY_H
