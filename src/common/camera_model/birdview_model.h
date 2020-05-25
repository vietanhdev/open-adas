#if !defined(BIRDVIEW_MODEL_H)
#define BIRDVIEW_MODEL_H

#include <opencv2/opencv.hpp>
#include <atomic>
#include "utils.h"

class BirdViewModel {

    int birdview_img_width = 1000;
    int birdview_img_height = 10000;
    FourPoints four_points =
        FourPoints(cv::Point2f(250, 8000), cv::Point2f(750, 8000),
                   cv::Point2f(750, 8500), cv::Point2f(250, 8500));
    FourPoints four_image_points;
    float width_pixel_to_meter_ratio = 0.1;
    float height_pixel_to_meter_ratio = 0.1;
    float car_y_in_pixel = 1000;
    float car_width_in_pixel = 750;
    std::atomic<bool> is_calibrated = false;

    cv::Mat birdview_transform_matrix;
    cv::Mat inv_birdview_transform_matrix;

    std::mutex mtx;

   public:
    BirdViewModel() {}

    void calibrate(float car_width, float carpet_width,
                   float car_to_carpet_distance, float carpet_length,
                   FourPoints four_image_points);
    cv::Mat transformImage(const cv::Mat &img);
    
    void transformPoints(const std::vector<cv::Point2f> &normalized_points, std::vector<cv::Point2f> &normalized_dst_points);

    float getDistanceToCar(float y);

    cv::Mat getDangerZone(const cv::Size img_size, float danger_distance);

};

#endif  // BIRDVIEW_MODEL_H
