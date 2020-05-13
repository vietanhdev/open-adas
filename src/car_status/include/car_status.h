#ifndef CAR_PROPS_H
#define CAR_PROPS_H

#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "lane_detector.h"
#include "object_detector.h"

class CarStatus {
   private:
    float speed;  // km/h

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

};
#endif