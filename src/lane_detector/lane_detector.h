#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <getopt.h>

#include <iostream>
#include <string>
#include <vector>

#include "lane_line.h"
#include "unet.h"

class LaneDetector {
   private:
    const int INPUT_WIDTH = 512;
    const int INPUT_HEIGHT = 512;
    const std::string INPUT_NODE = "data";
    const std::string OUTPUT_NODE = "sigmoid/Sigmoid";

    std::shared_ptr<Unet> model;

   public:
    bool ready = false;

    LaneDetector();

    cv::Mat getLaneMask(const cv::Mat& input_img);

    // Lane detect function
    // For debug purpose
    std::vector<LaneLine> detectLaneLines(const cv::Mat& input_img,
                                                cv::Mat& line_mask,
                                                cv::Mat& detected_lines_img,
                                                cv::Mat& reduced_lines_img);
    // For general usage
    std::vector<LaneLine> detectLaneLines(const cv::Mat& img);

   private:
    // Utils functions
    std::vector<cv::Vec4i> detectAndReduceLines(const cv::Mat& img,
                                                cv::Mat& detectedLinesImg,
                                                cv::Mat& reducedLinesImg);
    static cv::Vec2d linearParameters(cv::Vec4i line);
    static cv::Vec4i extendedLine(cv::Vec4i line, double d);
    static std::vector<cv::Point2i> boundingRectangleContour(cv::Vec4i line,
                                                             float d);
    static bool extendedBoundingRectangleLineEquivalence(
        const cv::Vec4i& _l1, const cv::Vec4i& _l2,
        float extensionLengthFraction, float maxAngleDiff,
        float boundingRectangleThickness);
};

#endif