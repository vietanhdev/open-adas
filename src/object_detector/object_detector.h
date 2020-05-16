#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <iostream>
#include <memory>
#include <string>

#include "ctdetNet.h"
#include "traffic_object.h"
#include "sign_classifier.h"

#include "filesystem_include.h"
#include "config.h"

class ObjectDetector {
   private:
    ctdet::ctdetNet * net;
    std::unique_ptr<float[]> outputData;

    TrafficSignClassifier sign_classifier;

   public:
    ObjectDetector();
    std::vector<TrafficObject> detect(const cv::Mat &img, const cv::Mat &original_img);
    void drawDetections(const std::vector<TrafficObject> & result,cv::Mat& img);
};

#endif  // OBJECT_DETECTOR_H
