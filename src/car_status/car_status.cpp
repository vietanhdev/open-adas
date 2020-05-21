#include "car_status.h"

using namespace std;

CarStatus::CarStatus() {
    std::lock_guard<std::mutex> guard(start_time_mutex);
    start_time = Timer::getCurrentTime();
}

void CarStatus::reset() {
    std::lock_guard<std::mutex> guard(start_time_mutex);
    std::lock_guard<std::mutex> guard2(speed_limit_mutex);
    start_time = Timer::getCurrentTime();
    speed_limit = MaxSpeedLimit();
}

Timer::time_point_t CarStatus::getStartTime() {
    std::lock_guard<std::mutex> guard(start_time_mutex);
    return start_time;
}

void CarStatus::setCurrentImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    cv::Mat resized = resizeByMaxSize(img, IMG_MAX_SIZE);
    current_img = resized;
    img.copyTo(current_img_origin_size);
}

cv::Mat CarStatus::getCurrentImage() {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.clone();
}

void CarStatus::getCurrentImage(cv::Mat &image) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.copyTo(image);
}

void CarStatus::getCurrentImage(cv::Mat &image, cv::Mat &original_image) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    current_img.copyTo(image);
    current_img_origin_size.copyTo(original_image);
}


void CarStatus::setDetectedObjects(const std::vector<TrafficObject> &objects) {
    std::lock_guard<std::mutex> guard(detected_objects_mutex);
    detected_objects = objects;
}

std::vector<TrafficObject> CarStatus::getDetectedObjects() {
    std::vector<TrafficObject> objects;
    std::lock_guard<std::mutex> guard(detected_objects_mutex);
    objects = detected_objects;
    return objects;
}


void CarStatus::setDetectedLaneLines(const std::vector<LaneLine> &lane_lines, 
    const cv::Mat& lane_line_mask,
    const cv::Mat& detected_line_img,
    const cv::Mat& reduced_line_img) 
{
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    detected_lane_lines = lane_lines;
    this->lane_line_mask = lane_line_mask;
    this->detected_line_img = detected_line_img;
    this->reduced_line_img = reduced_line_img;

}

void CarStatus::setDetectedLaneLines(const std::vector<LaneLine> &lane_lines) 
{
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    detected_lane_lines = lane_lines;
}

std::vector<LaneLine>  CarStatus::getDetectedLaneLines() 
{
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    std::vector<LaneLine> lane_lines = detected_lane_lines;
    return lane_lines;
}

cv::Mat CarStatus::getLineMask() {
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    return lane_line_mask.clone();
}

cv::Mat CarStatus::getDetectedLinesViz() {
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    return detected_line_img.clone();
}

cv::Mat CarStatus::getReducedLinesViz() {
    std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
    return reduced_line_img.clone();
}


float CarStatus::getCarSpeed() {
    return car_speed;
}

float CarStatus::getDangerDistance() {
    return getCarSpeed() / 3.6 * 2.0;
}

void CarStatus::setCollisionWarning(bool is_warning) {
    std::lock_guard<std::mutex> guard(collision_warning_status_mutex);
    if (is_warning) {
        if (!collision_warning_status.is_warning) {
            collision_warning_status.is_warning = true;
            collision_warning_status.begin_time = Timer::getCurrentTime();
            collision_warning_status.should_notify = true;
        }
    } else {
        collision_warning_status.is_warning = false;
    }
}

CollisionWarningStatus CarStatus::getCollisionWarning() {
    std::lock_guard<std::mutex> guard(collision_warning_status_mutex);

    if (collision_warning_status.is_warning && \
        Timer::calcTimePassed(collision_warning_status.notified_time) > COLLISION_WARNING_INTERVAL) {
        collision_warning_status.should_notify = true;
        collision_warning_status.notified_time = Timer::getCurrentTime();
    }

    CollisionWarningStatus ret_collision_warning_status = collision_warning_status;

    if (collision_warning_status.is_warning) {
        collision_warning_status.should_notify = false;
    }

    return ret_collision_warning_status; 
}

void CarStatus::setCarSpeed(float speed) {
    car_speed = speed;
}

cv::Mat CarStatus::resizeByMaxSize(const cv::Mat &img, int max_size) {

    int width = img.cols;
    int height = img.rows;

    if ((width < max_size && height < max_size) || max_size <= 0) {
        return img;
    }

    if (width > height) {
        float resize_ratio = (float)max_size / width;
        cv::Mat resized_img;
        cv::resize(img, resized_img, cv::Size(), resize_ratio, resize_ratio);
        return resized_img;
    } else {
        float resize_ratio = (float)max_size / height;
        cv::Mat resized_img;
        cv::resize(img, resized_img, cv::Size(), resize_ratio, resize_ratio);
        return resized_img;
    }

}


void CarStatus::setObjectDetectionTime(Timer::time_duration_t duration) {
    std::lock_guard<std::mutex> guard(time_mutex);
    object_detection_time = duration;
}

Timer::time_duration_t CarStatus::getObjectDetectionTime() {
    std::lock_guard<std::mutex> guard(time_mutex);
    return object_detection_time;
}

void CarStatus::setLaneDetectionTime(Timer::time_duration_t duration) {
    std::lock_guard<std::mutex> guard(time_mutex);
    lane_detection_time = duration;
}

Timer::time_duration_t CarStatus::getLaneDetectionTime() {
    std::lock_guard<std::mutex> guard(time_mutex);
    return lane_detection_time;
}

// Get max speed limit
MaxSpeedLimit CarStatus::getMaxSpeedLimit() {
    std::lock_guard<std::mutex> guard(speed_limit_mutex);

    // Turn off speed limit if expired
    if (Timer::calcTimePassed(speed_limit.begin_time) > MAX_SPEED_SIGN_VALID_TIME) {
        speed_limit.speed_limit = -1;
    }

    // Check for overspeed
    if (speed_limit.speed_limit > 0 &&
        getCarSpeed() > speed_limit.speed_limit &&
        !speed_limit.overspeed_warning && 
        Timer::calcTimePassed(speed_limit.begin_time) >  OVERSPEED_WARNING_AFTER_TRAFFIC_SIGN
    ) {
        speed_limit.overspeed_warning = true;
        speed_limit.overspeed_warning_has_notified = false;
        speed_limit.overspeed_warning_notified_time = Timer::getCurrentTime();
    
    // If the car speed is become normal, 
    // turn off warning
    } else if (getCarSpeed() <= speed_limit.speed_limit) {
        speed_limit.overspeed_warning = false;

    // Turn on notification again if last notification time
    // is larger than SPEED_WARNING_INTERVAL
    } else if (speed_limit.overspeed_warning_has_notified && 
       Timer::calcTimePassed(speed_limit.overspeed_warning_notified_time) > OVERSPEED_WARNING_INTERVAL ) {
        speed_limit.overspeed_warning_has_notified = false;
    }

    MaxSpeedLimit ret_speed_limit = speed_limit;

    // After return a speed limit, turn all notification off
    speed_limit.has_notified = true;
    speed_limit.overspeed_warning_has_notified = true;

    return ret_speed_limit;
}

void CarStatus::removeSpeedLimit() {
    std::lock_guard<std::mutex> guard(speed_limit_mutex);
    speed_limit.has_notified = false;
    speed_limit.speed_limit = 0;
    cout << "END OF SPEED LIMIT" << endl;
}

void CarStatus::triggerSpeedLimit(int speed) {
    std::lock_guard<std::mutex> guard(speed_limit_mutex);

    if (speed != speed_limit.speed_limit || 
        Timer::calcTimePassed(speed_limit.begin_time) > TIME_TO_RENOTIFY_A_SAME_TRAFFIC_SIGN) {
        speed_limit.has_notified = false;
        speed_limit.speed_limit = speed;
        speed_limit.begin_time = Timer::getCurrentTime();
        cout << "MAX SPEED LIMIT: " << speed << endl;
    }

}
