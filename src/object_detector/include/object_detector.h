#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <iostream>
#include <memory>
#include <string>

#include "ctdetNet.h"
#include "ctdet_utils.h"

class ObjectDetector {
   private:
    ctdet::ctdetNet * net;
    std::unique_ptr<float[]> outputData;

   public:
    ObjectDetector();
    std::vector<Detection> inference(const cv::Mat &img);
};

#endif  // OBJECT_DETECTOR_H
