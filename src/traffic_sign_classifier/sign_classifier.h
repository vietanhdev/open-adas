#ifndef SIGN_CLASSIFIER_H
#define SIGN_CLASSIFIER_H

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "traffic_sign.h"
#include "classification_net.h"

class TrafficSignClassifier {
   private:
    const int INPUT_WIDTH = 64;
    const int INPUT_HEIGHT = 64;
    const std::string INPUT_NODE = "resnet50_input";
    const std::string OUTPUT_NODE = "dense/Softmax";

    std::shared_ptr<ClassificationNet> model;

   public:
    bool ready = false;

    TrafficSignClassifier();

    int getSignId(const cv::Mat& input_img);
    std::string getSignName(const cv::Mat& input_img);
    std::string getSignName(int class_id);

};

#endif