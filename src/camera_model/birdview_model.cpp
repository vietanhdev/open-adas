#include "birdview_model.h"

using namespace std;

void BirdViewModel::calibrate(float car_width, float carpet_width,
               float car_to_carpet_distance, float carpet_length,
               FourPoints four_image_points) {

    std::lock_guard<std::mutex> guard(mtx);

    width_pixel_to_meter_ratio =
        carpet_width / (four_points.tr.x - four_points.tl.x);
    height_pixel_to_meter_ratio =
        carpet_length / (four_points.br.y - four_points.tr.y);
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


void BirdViewModel::transformPoints(const std::vector<cv::Point2f> &normalized_points, std::vector<cv::Point2f> &normalized_dst_points) {
    std::lock_guard<std::mutex> guard(mtx);

    birdview_transform_matrix = cv::getPerspectiveTransform(
        four_image_points.to_vector(),
        four_points.to_vector());

    perspectiveTransform(normalized_points, normalized_dst_points, birdview_transform_matrix);

}

float BirdViewModel::getDistanceToCar(float y) {
    std::lock_guard<std::mutex> guard(mtx);
    float distance = (car_y_in_pixel - y) * height_pixel_to_meter_ratio;
    if (distance < 0) {
        distance = -1;
    }
    return distance;
}

cv::Mat BirdViewModel::getDangerZone(const cv::Size img_size, float danger_distance) {

    std::lock_guard<std::mutex> guard(mtx);
    int tl_y = car_y_in_pixel - danger_distance / height_pixel_to_meter_ratio;
    int tl_x = birdview_img_width / 2 - car_width_in_pixel / 2;
    int br_x = tl_x + car_width_in_pixel;
    int br_y = car_y_in_pixel - 1;

    cv::Mat danger_mask(cv::Size(birdview_img_width, birdview_img_height), CV_8UC1, cv::Scalar(0));
    cv::rectangle(danger_mask, cv::Rect(tl_x, tl_y, br_x-tl_x, br_y-tl_y), 
        cv::Scalar(255), -1);

    cv::Mat transformed_danger_mask;
    cv::Mat inv_birdview_transform_matrix = cv::getPerspectiveTransform(
        four_points.to_vector(),
        four_image_points.to_vector(img_size.width, img_size.height)
    );

    cv::warpPerspective(danger_mask, transformed_danger_mask, inv_birdview_transform_matrix, img_size);

    cv::threshold(danger_mask, danger_mask, 1, 255, cv::THRESH_BINARY);

    return transformed_danger_mask;
}