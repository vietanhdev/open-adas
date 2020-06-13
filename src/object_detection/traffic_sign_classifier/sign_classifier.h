#ifndef SIGN_CLASSIFIER_H
#define SIGN_CLASSIFIER_H

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h> 

#include "traffic_sign.h"
#include "classification_net.h"

class TrafficSignClassifier {

   private:
    UffModelParams params;
    std::shared_ptr<ClassificationNet> model;

   public:
    bool ready = false;

    TrafficSignClassifier();

    std::vector<int> getSignIds(const std::vector<cv::Mat>& input_img);
    std::vector<std::string> getSignNames(const std::vector<cv::Mat>& input_imgs);
    std::vector<std::string> getSignNames(std::vector<int>& class_ids);
    std::string getSignName(int class_id);
    static bool isSpeedSign(std::string sign_name);

};

#endif