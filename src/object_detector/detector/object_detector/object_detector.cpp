#include "object_detector.h"

ObjectDetector::ObjectDetector(std::string model_path) {
    net = new ctdet::ctdetNet(model_path);
    outputData = std::unique_ptr<float[]>(new float[net->outputBufferSize]);
}

std::vector<Detection> ObjectDetector::inference(const cv::Mat &img) {

    cv::Mat frame(img);
    auto inputData = prepareImage(frame, net->forwardFace);

    net->doInference(inputData.data(), outputData.get());
    net->printTime();

    int num_det = static_cast<int>(outputData[0]);

    std::vector<Detection> result;
    result.resize(num_det);
    memcpy(result.data(), &outputData[1], num_det * sizeof(Detection));

    postProcess(result, img, net->forwardFace);

    return result;
}