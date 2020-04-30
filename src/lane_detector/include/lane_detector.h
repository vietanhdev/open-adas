#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <iostream>
#include <string>
#include <getopt.h>
#include <vector>
#include "unet.h"

class LaneDetector {
   private:

    const int INPUT_WIDTH = 320;
    const int INPUT_HEIGHT = 224;
    const std::string INPUT_NODE = "data";
    const std::string OUTPUT_NODE = "sigmoid/Sigmoid";
    
    std::shared_ptr<Unet> model;

   public:
    bool ready = false;


    LaneDetector();

    cv::Mat detectLane(const cv::Mat& img);

};


#endif