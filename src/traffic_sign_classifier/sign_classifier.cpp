#include "sign_classifier.h"

#include "../config.h"

using namespace cv;

TrafficSignClassifier::TrafficSignClassifier() {
    UffModelParams params;
    params.inputW = INPUT_WIDTH;
    params.inputH = INPUT_HEIGHT;
    params.batchSize = 1;
    params.nClasses = 16;
    params.uffFilePath = SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_MODEL;
    params.engineFilePath = SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_TENSORRT_PLAN;
    params.classListFile = SMARTCAM_TRAFFIC_SIGN_CLASS_LIST;
    params.forceRebuildEngine = false;
    params.inputTensorNames.push_back(INPUT_NODE);
    params.outputTensorNames.push_back(OUTPUT_NODE);
    params.fp16 = false;
    params.int8 = false;

    auto test = gLogger.defineTest("TrafficSignClassifier", 0, NULL);
    gLogger.reportTestStart(test);

    model = std::make_shared<ClassificationNet>(params);
}

int TrafficSignClassifier::getSignId(const cv::Mat& input_img) {
    int result;
    if (!model->infer(input_img, result)) {
        cerr << "Error on running traffic sign classification model." << endl;
    }
    return result;
}

std::string TrafficSignClassifier::getSignName(const cv::Mat& input_img) {
    int result;
    if (!model->infer(input_img, result)) {
        cerr << "Error on running traffic sign classification model." << endl;
    }
    return model->getClassName(result);
}
