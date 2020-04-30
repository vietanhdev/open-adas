#include "../config.h"
#include "lane_detector.h"

LaneDetector::LaneDetector() {

    UnetParams params;
    params.inputW = INPUT_WIDTH;
    params.inputH = INPUT_HEIGHT;
    params.batchSize = 1;
    params.nClasses = 1;
    params.uffFilePath = SMARTCAM_LANE_DETECTION_MODEL;
    params.engineFilePath = SMARTCAM_LANE_DETECTION_TENSORRT_PLAN;
    params.forceRebuildEngine = false;
    params.inputTensorNames.push_back(INPUT_NODE);
    params.outputTensorNames.push_back(OUTPUT_NODE);

    auto test = gLogger.defineTest("LaneDetector", 0, NULL);
    gLogger.reportTestStart(test);

    model = std::make_shared<Unet>(params);

}

cv::Mat LaneDetector::detectLane(const cv::Mat& input_img) {
    cv::Mat output_img;
    if (!model->infer(input_img, output_img)) {
        cerr << "Error on running lane detection model." << endl;
    }
    return output_img;
}