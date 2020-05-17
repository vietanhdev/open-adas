#include "sign_classifier.h"

#include "config.h"

using namespace cv;

TrafficSignClassifier::TrafficSignClassifier() {
    UffModelParams params;
    params.inputW = INPUT_WIDTH;
    params.inputH = INPUT_HEIGHT;
    params.batchSize = 1;
    params.nClasses = 15;
    params.uffFilePath = SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_MODEL;
    params.engineFilePath = SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_TENSORRT_PLAN;
    params.classListFile = SMARTCAM_TRAFFIC_SIGN_CLASS_LIST;
    params.forceRebuildEngine = false;
    params.inputTensorNames.push_back(INPUT_NODE);
    params.outputTensorNames.push_back(OUTPUT_NODE);
    params.fp16 = true;
    params.int8 = false;

    auto test = gLogger.defineTest("TrafficSignClassifier", 0, NULL);
    gLogger.reportTestStart(test);

    model = std::make_shared<ClassificationNet>(params);
}

int TrafficSignClassifier::getSignId(const cv::Mat& input_img) {
    int result;
    if (!model->infer(input_img, result, SIGN_CLASSIFICATION_THRESH)) {
        cerr << "Error on running traffic sign classification model." << endl;
    }
    return result;
}

std::string TrafficSignClassifier::getSignName(const cv::Mat& input_img) {
    return model->getClassName(getSignId(input_img));
}

std::string TrafficSignClassifier::getSignName(int class_id) {
    return model->getClassName(class_id);
}

bool TrafficSignClassifier::isSpeedSign(std::string sign_name) {
    if (sign_name.find("SPEED_LIMIT") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
