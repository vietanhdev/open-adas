#include "birdview_model.h"

void BirdViewModel::calibrate(float car_width, float carpet_width,
               float car_to_carpet_distance, float carpet_length,
               FourPoints four_image_points) {
    std::lock_guard<std::mutex> guard(mtx);

    width_pixel_to_meter_ratio =
        carpet_width / four_points.tr.x - four_points.tl.x;
    height_pixel_to_meter_ratio =
        carpet_length / four_points.br.y - four_points.tr.y;
    car_y_in_pixel =
        four_points.br.y + car_to_carpet_distance / height_pixel_to_meter_ratio;
    car_width_in_pixel = car_width / width_pixel_to_meter_ratio;

    this->four_image_points = four_image_points;
    is_calibrated = true;
}

cv::Mat BirdViewModel::transformImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(mtx);
    cv::Mat dst;

    int img_width = img.cols;
    int img_height = img.rows;

    inv_birdview_transform_matrix = cv::getPerspectiveTransform(
        four_points.to_vector(),
        four_image_points.to_vector(img_width, img_height));

    birdview_transform_matrix = cv::getPerspectiveTransform(
        four_image_points.to_vector(img_width, img_height),
        four_points.to_vector());

    std::vector<cv::Point2f> points =
        four_image_points.to_vector(img_width, img_height);

    cv::warpPerspective(img, dst, birdview_transform_matrix,
                        cv::Size(birdview_img_width, birdview_img_height));
    return dst;
}