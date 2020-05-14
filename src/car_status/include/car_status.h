#ifndef CAR_PROPS_H
#define CAR_PROPS_H

#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <atomic>

#include "lane_detector.h"
#include "object_detector.h"
#include "timer.h"

class CarStatus {
   private:

    // Current image
    cv::Mat current_img;
    std::mutex current_img_mutex;

    // Lane detection result
    cv::Mat lane_line_mask;
    cv::Mat detected_line_img;
    cv::Mat reduced_line_img;
    std::vector<LaneLine> detected_lane_lines;
    std::mutex lane_detection_results_mutex;

    // Object detection result
    std::vector<Detection> detected_objects;
    std::mutex detected_objects_mutex;

    std::atomic<float> car_speed; // km/h

    // Time durations
    std::mutex time_mutex;
    Timer::time_duration_t object_detection_time;
    Timer::time_duration_t lane_detection_time;

   public:
    void setCurrentImage(const cv::Mat &img);
    cv::Mat getCurrentImage();

    void setDetectedObjects(const std::vector<Detection> &objects);
    std::vector<Detection> getDetectedObjects();

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
    void setCarSpeed(float speed);

    cv::Mat resizeByMaxSize(const cv::Mat &img, int max_size);

    void setObjectDetectionTime(Timer::time_duration_t duration);
    Timer::time_duration_t getObjectDetectionTime();
    void setLaneDetectionTime(Timer::time_duration_t duration);
    Timer::time_duration_t getLaneDetectionTime();

};
#endif