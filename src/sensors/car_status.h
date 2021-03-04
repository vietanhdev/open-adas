#ifndef CAR_STATUS_H
#define CAR_STATUS_H

#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <atomic>
#include <iostream>

#include "perception/lane_detection/lane_detector.h"
#include "perception/object_detection/object_detector.h"

#include "sensors/collision_warning_status.h"

#include "sensors/speed_limit.h"

#include "utils/timer.h"

class CarStatus {
   private:

    // Start time of this car status
    // Reset when calling reset()
    Timer::time_point_t start_time;
    std::mutex start_time_mutex;

    // Current image
    cv::Mat current_img;
    cv::Mat current_img_origin_size;
    std::mutex current_img_mutex;

    // Lane detection result
    cv::Mat lane_line_mask;
    cv::Mat detected_line_img;
    cv::Mat reduced_line_img;
    std::vector<LaneLine> detected_lane_lines;
    std::mutex lane_detection_results_mutex;

    // Object detection result
    std::vector<TrafficObject> detected_objects;
    std::mutex detected_objects_mutex;

    std::atomic<float> car_speed; // km/h
    std::atomic<bool> turning_left;
    std::atomic<bool> turning_right;
    Timer::time_point_t last_activated_turning_signal_time;
    std::mutex last_activated_turning_signal_time_mutex;

    // Time durations
    std::mutex time_mutex;
    Timer::time_duration_t object_detection_time;
    Timer::time_duration_t lane_detection_time;

    // Speed limit
    MaxSpeedLimit speed_limit;
    std::mutex speed_limit_mutex;

    // Collision warning
    CollisionWarningStatus collision_warning_status;
    std::mutex collision_warning_status_mutex;

   public:

    CarStatus();
    void reset();
    Timer::time_point_t getStartTime();

    void setCurrentImage(const cv::Mat &img);
    void getCurrentImage(cv::Mat &image, cv::Mat &original_image);
    cv::Mat getCurrentImage();
    void getCurrentImage(cv::Mat &image); // Better performance

    void setDetectedObjects(const std::vector<TrafficObject> &objects);
    std::vector<TrafficObject> getDetectedObjects();

    void setDetectedLaneLines(const std::vector<LaneLine> &lane_lines, 
        const cv::Mat& lane_line_mask,
        const cv::Mat& detected_line_img,
        const cv::Mat& reduced_line_img);

    void setDetectedLaneLines(const std::vector<LaneLine> &lane_lines);
    std::vector<LaneLine>  getDetectedLaneLines();

    cv::Mat getLineMask();
    cv::Mat getDetectedLinesViz();
    cv::Mat getReducedLinesViz();

    float getCarSpeed();
    bool getLeftTurnSignal();
    bool getRightTurnSignal();
    Timer::time_point_t getLastActivatedTurningSignalTime();
    float getDangerDistance();
    void setCarSpeed(float speed);
    void setCarStatus(float speed, bool turning_left, bool turning_right);

    void setCollisionWarning(bool is_warning);
    CollisionWarningStatus getCollisionWarning();

    cv::Mat resizeByMaxSize(const cv::Mat &img, int max_size);

    void setObjectDetectionTime(Timer::time_duration_t duration);
    Timer::time_duration_t getObjectDetectionTime();
    void setLaneDetectionTime(Timer::time_duration_t duration);
    Timer::time_duration_t getLaneDetectionTime();

    MaxSpeedLimit getMaxSpeedLimit();
    void removeSpeedLimit();
    void triggerSpeedLimit(int speed);

};
#endif