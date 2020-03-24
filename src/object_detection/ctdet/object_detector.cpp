#include "object_detector.h"

ObjectDetector::ObjectDetector() {
    net = new ctdet::ctdetNet("models/object_detection/ctdet_coco_resdcn18.engine");
    outputData = std::unique_ptr<float[]>(new float[net->outputBufferSize]);
}

cv::Mat ObjectDetector::inference(const cv::Mat &img) {

    cv::Mat frame(img);
    auto inputData = prepareImage(frame, net->forwardFace);

    net->doInference(inputData.data(), outputData.get());
    net->printTime();

    int num_det = static_cast<int>(outputData[0]);

    std::vector<Detection> result;

    result.resize(num_det);

    memcpy(result.data(), &outputData[1], num_det * sizeof(Detection));

    postProcess(result, img, net->forwardFace);

    cv::RNG rng(244);
    std::vector<cv::Scalar> color = { cv::Scalar(255, 0,0),cv::Scalar(0, 255,0)};
    drawImg(result, frame, color, net->forwardFace);

    return frame;
}