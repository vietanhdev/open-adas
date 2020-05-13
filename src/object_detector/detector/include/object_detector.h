#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <iostream>
#include <memory>
#include <string>

#include "filesystem_include.h"
#include "ctdetNet.h"
#include "ctdet_utils.h"

class ObjectDetector {
   private:
    ctdet::ctdetNet * net;
    std::unique_ptr<float[]> outputData;

   public:
    ObjectDetector(std::string onnx_model_path, std::string tensorrt_plan_path, std::string tensorrt_mode="FLOAT16");
    std::vector<Detection> inference(const cv::Mat &img);
};

#endif  // OBJECT_DETECTOR_H
