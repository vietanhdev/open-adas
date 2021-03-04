#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <iostream>
#include <memory>
#include <string>

#include "../common/onnx_models/include/ctdetNet.h"
#include "traffic_object.h"
#include "traffic_sign_classification/sign_classifier.h"

#include "utils/filesystem_include.h"
#include "configs/config.h"

class ObjectDetector {
   private:
    ctdet::ctdetNet * net;
    std::unique_ptr<float[]> outputData;

    TrafficSignClassifier sign_classifier;

   public:
    ObjectDetector();
    std::vector<TrafficObject> detect(const cv::Mat &img, const cv::Mat &original_img);
    void drawDetections(const std::vector<TrafficObject> & result,cv::Mat& img);

    bool isInStrVector(const std::string &value, const std::vector<std::string> &array);
};

#endif  // OBJECT_DETECTOR_H
