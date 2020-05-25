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
bool ClassificationNet::processInput(const samplesCommon::BufferManager& buffers, const std::vector<cv::Mat> & imgs) {
    const int inputH = mParams.inputH;
    const int inputW = mParams.inputW;
    const int batchSize = imgs.size();

    // put data into buffer
    float* hostDataBuffer =
        static_cast<float*>(buffers.getHostBuffer(mParams.inputTensorNames[0]));

    int volChl = inputH * inputW;
    int volImg = volChl * 3;
    for (int i = 0; i < batchSize; ++i) {

        cv::Mat resized_img;
        cv::resize(imgs[i], resized_img, cv::Size(inputW, inputH));
        resized_img.convertTo(resized_img, CV_32FC3, 1.0/255);

        for (int x = 0; x < inputW; ++x) {
            for (int y = 0; y < inputH; ++y) {
                cv::Vec3f intensity = resized_img.at<cv::Vec3f>(y, x);

                int j = y * inputH + x;

                hostDataBuffer[i * volImg + 0 * volChl + j] = float(intensity.val[2]);
                hostDataBuffer[i * volImg + 1 * volChl + j] = float(intensity.val[1]);
                hostDataBuffer[i * volImg + 2 * volChl + j] = float(intensity.val[0]);
            }
        }
    }

    return true;
}

// Process output and verify result
bool ClassificationNet::processOutput(const samplesCommon::BufferManager& buffers, std::vector<int> & labels, int n_samples, float threshold) {

    const float* out_buff = static_cast<float*>(
        buffers.getHostBuffer(mParams.outputTensorNames[0]));

    for (int batch = 0; batch < n_samples; ++batch) {
        int label = -1;
        float max_prob = 0.0;
        for (int i = 0; i < mParams.nClasses; ++i) {
            if (out_buff[i] > max_prob) {
                label = i;
                max_prob = out_buff[batch * mParams.nClasses + i];
            }
        }

        if (max_prob < threshold) {
            label = -1;
        }

        labels.push_back(label);
    }

    return true;
}

// Runs the TensorRT inference engine
// This function is the main execution function
// It allocates the buffer, sets inputs and executes the engine
bool ClassificationNet::infer(const std::vector<cv::Mat>& input_imgs, std::vector<int>& labels, float threshold) {

    if (!processInput(*buffers, input_imgs)) {
        cout << "Processing input failed" << endl;
        return false;
    }

    // Memcpy from host input buffers to device input buffers
    buffers->copyInputToDevice();

    bool status = context->execute(input_imgs.size(),
                                   buffers->getDeviceBindings().data());
    if (!status) {
        cout << "CUDA execution failed" << endl;
        return false;
    }

    // Memcpy from device output buffers to host output buffers
    buffers->copyOutputToHost();

    // Post-process output
    if (!processOutput(*buffers, labels, input_imgs.size(), threshold)) {
        cout << "Processing output failed" << endl;
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