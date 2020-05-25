#include "collision_warning_controller.h"


using namespace std;

CollisionWarningController::CollisionWarningController(std::shared_ptr<CameraModel> camera_model, 
    std::shared_ptr<CarStatus> car_status
) {
    this->camera_model = camera_model;
    this->car_status = car_status;
}

void CollisionWarningController::processingThread(CollisionWarningController *this_ptr) {
    cv::Mat img;
    std::vector<TrafficObject> objects;
    while (true) {
        {
            std::unique_lock<std::mutex> lck(this_ptr->mtx);
            while (!this_ptr->new_data_available) this_ptr->cv.wait(lck);
            this_ptr->img.copyTo(img);
            objects = this_ptr->objects;
            this_ptr->new_data_available = false;
        }

        cv::Mat transform_img = this_ptr->camera_model->getBirdViewModel()->transformImage(img);
        cv::imwrite("debug_tranform.png", transform_img);

    }
}

void CollisionWarningController::updateData(const cv::Mat &img, const std::vector<TrafficObject> &objects) {
    std::unique_lock<std::mutex> lck(mtx);
    img.copyTo(this->img);
    this->objects = objects;
    new_data_available = true;
    cv.notify_all();
}

void CollisionWarningController::calculateDistance(const cv::Mat &img, std::vector<TrafficObject> &objects) {

    // Check transform
    // cv::Mat transform_img = camera_model->getBirdViewModel()->transformImage(img);
    // cv::imwrite("debug_tranform.png", transform_img);

    int img_width = img.cols;
    int img_height = img.rows;
    for (size_t i = 0; i < objects.size(); ++i) {
        std::vector<cv::Point2f> points;
        points.push_back(cv::Point2f(
            static_cast<float>(objects[i].bbox.x2) / img_width, 
            static_cast<float>(objects[i].bbox.y2) / img_height));
        points.push_back(cv::Point2f(
            static_cast<float>(objects[i].bbox.x1) / img_width, 
            static_cast<float>(objects[i].bbox.y2) / img_height));
        std::vector<cv::Point2f> transformed_points;
        camera_model->getBirdViewModel()->transformPoints(points, transformed_points);

        float max_y = max({transformed_points[0].y,
            transformed_points[1].y
        });

        float distance = camera_model->getBirdViewModel()->getDistanceToCar(max_y);
        objects[i].distance_to_my_car = distance;
    }
}


bool CollisionWarningController::isInDangerSituation(const cv::Size &img_size,        
    std::vector<TrafficObject> &objects) {
        
    if (car_status->getCarSpeed() < MIN_SPEED_FOR_COLLISION_WARNING) {
        return false;
    }

    float danger_distance = car_status->getDangerDistance();
    cv::Mat danger_zone = camera_model->getBirdViewModel()->getDangerZone(img_size, danger_distance);

    cv::Mat object_mask(img_size, CV_8UC1, cv::Scalar(0));
    for (size_t i = 0; i < objects.size(); ++i) {
        cv::rectangle(object_mask, cv::Rect(
            cv::Point(objects[i].bbox.x1, objects[i].bbox.y1), 
            cv::Point(objects[i].bbox.x2, objects[i].bbox.y2)),
            cv::Scalar(255), -1);
    }

    cv::Mat in_danger;
    cv::bitwise_and(danger_zone, object_mask, in_danger);

    if (cv::countNonZero(in_danger) > 20) {
        return true;
    }

    return false;
    
}