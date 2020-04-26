#include "object_detector.h"

using namespace std;

ObjectDetector::ObjectDetector(std::string onnx_model_path, std::string tensorrt_plan_path, std::string tensorrt_mode) {

    // If plan file is existed => Load it
    if (fs::exists(tensorrt_plan_path)) {

        cout << "Loading TensorRT plan file at: " << tensorrt_plan_path << endl;
        net = new ctdet::ctdetNet(tensorrt_plan_path);

    } else { // Else, create engine file

        cout << "TensorRT plan file not found. Creating a new plan file at: " << tensorrt_plan_path << endl;
        if (tensorrt_mode == "FLOAT32") {
            net = new ctdet::ctdetNet(onnx_model_path, "", ctdet::RUN_MODE::FLOAT32);
        } else if (tensorrt_mode == "FLOAT16") {
            net = new ctdet::ctdetNet(onnx_model_path, "", ctdet::RUN_MODE::FLOAT16);
        } else {
            cout << "TensorRT mode " << tensorrt_mode << " is not supported now. Please build model using `build_tensorrt_engine`" << endl;
        }
        net->saveEngine(tensorrt_plan_path);

    }
    
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