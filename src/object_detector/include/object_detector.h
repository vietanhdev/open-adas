#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <iostream>
#include <memory>
#include <string>

#include "filesystem_include.h"
#include "ctdetNet.h"
#include "ctdet_utils.h"
#include "config.h"

class ObjectDetector {
   private:
    ctdet::ctdetNet * net;
    std::unique_ptr<float[]> outputData;

   public:
    ObjectDetector();
    std::vector<Detection> detect(const cv::Mat &img);
    void drawDetections(const std::vector<Detection> & result,cv::Mat& img);
};

#endif  // OBJECT_DETECTOR_H
