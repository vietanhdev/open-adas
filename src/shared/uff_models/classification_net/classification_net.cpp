#include "classification_net.h"

#include <cuda_runtime_api.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>

#include "BatchStream.h"
#include "EntropyCalibrator.h"
#include "NvInfer.h"
#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

ClassificationNet::ClassificationNet(const UffModelParams& params): UffModel(params) {}

// Reads the input, pre-process, and stores in managed buffer
bool ClassificationNet::processInput(const samplesCommon::BufferManager& buffers, const cv::Mat& img) {
    int inputH = mParams.inputH;
    int inputW = mParams.inputW;

    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(inputW, inputH));
    resized_img.convertTo(resized_img, CV_32FC3, 1.0/255);

    // put data into buffer
    float* hostDataBuffer =
        static_cast<float*>(buffers.getHostBuffer(mParams.inputTensorNames[0]));

    int volChl = inputH * inputW;

    for (int x = 0; x < inputW; ++x) {
        for (int y = 0; y < inputH; ++y) {
            cv::Vec3f intensity = resized_img.at<cv::Vec3f>(y, x);
            float blue = intensity.val[0];
            float green = intensity.val[1];
            float red = intensity.val[2];

            int j = y * inputH + x;

            hostDataBuffer[0 * volChl + j] = float(blue);
            hostDataBuffer[1 * volChl + j] = float(green);
            hostDataBuffer[2 * volChl + j] = float(red);
        }
    }

    return true;
}

// Process output and verify result
bool ClassificationNet::processOutput(const samplesCommon::BufferManager& buffers, int & object_class, float threshold) {

    const float* out_buff = static_cast<float*>(
        buffers.getHostBuffer(mParams.outputTensorNames[0]));

    object_class = -1;
    float max_prob = 0.0;
    for (int i = 0; i < mParams.nClasses; ++i) {
        if (out_buff[i] > max_prob) {
            object_class = i;
            max_prob = out_buff[i];
        }
    }

    if (max_prob < threshold) {
        object_class = -1;
    }

    return true;
}

// Runs the TensorRT inference engine
// This function is the main execution function
// It allocates the buffer, sets inputs and executes the engine
bool ClassificationNet::infer(const cv::Mat& input_img, int& object_class, float threshold) {
    int origin_w = input_img.size().width;
    int origin_h = input_img.size().height;

    if (!processInput(*buffers, input_img)) {
        return false;
    }

    // Memcpy from host input buffers to device input buffers
    buffers->copyInputToDevice();

    bool status = context->execute(mParams.batchSize,
                                   buffers->getDeviceBindings().data());
    if (!status) {
        return false;
    }

    // Memcpy from device output buffers to host output buffers
    buffers->copyOutputToHost();

    // Post-process output
    if (!processOutput(*buffers, object_class, threshold)) {
        return false;
    }

    return true;
}

std::string ClassificationNet::getClassName(int class_id) {
    for (size_t i = 0; i < mParams.classes.size(); ++i) {
        if (mParams.classes[i].id == class_id) {
            return mParams.classes[i].name;
        }
    }
    return "";
}