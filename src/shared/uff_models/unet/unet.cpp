#include "unet.h"

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

Unet::Unet(const UffModelParams& params): UffModel(params) {}

// Reads the input, pre-process, and stores in managed buffer
bool Unet::processInput(const samplesCommon::BufferManager& buffers,
                        const cv::Mat& img) {
    int inputH = mParams.inputH;
    int inputW = mParams.inputW;

    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(inputW, inputH));

    // put data into buffer
    float* hostDataBuffer =
        static_cast<float*>(buffers.getHostBuffer(mParams.inputTensorNames[0]));

    int volChl = inputH * inputW;

    for (int x = 0; x < inputW; ++x) {
        for (int y = 0; y < inputH; ++y) {
            cv::Vec3b intensity = resized_img.at<cv::Vec3b>(y, x);
            int blue = intensity.val[0];
            int green = intensity.val[1];
            int red = intensity.val[2];

            int j = y * inputH + x;

            hostDataBuffer[0 * volChl + j] = float(red);
            hostDataBuffer[1 * volChl + j] = float(green);
            hostDataBuffer[2 * volChl + j] = float(blue);
        }
    }

    return true;
}

// Process output and verify result
bool Unet::processOutput(const samplesCommon::BufferManager& buffers,
                         cv::Mat& output_img) {
    int inputH = mParams.inputH;
    int inputW = mParams.inputW;

    const float* out_buff = static_cast<float*>(
        buffers.getHostBuffer(mParams.outputTensorNames[0]));

    output_img = cv::Mat(inputH, inputW, CV_32F, cv::Scalar(0));

    for (int x = 0; x < inputW; ++x) {
        for (int y = 0; y < inputH; ++y) {
            int j = y * inputH + x;
            output_img.at<float>(y, x) = out_buff[j];
        }
    }

    return true;
}

// Runs the TensorRT inference engine
// This function is the main execution function
// It allocates the buffer, sets inputs and executes the engine
bool Unet::infer(const cv::Mat& input_img, cv::Mat& output_img) {
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

    cv::Mat prepared_output;

    // Post-process output
    if (!processOutput(*buffers, prepared_output)) {
        return false;
    }

    // Resize output_img to original size
    cv::resize(prepared_output, prepared_output, cv::Size(origin_w, origin_h));

    output_img = prepared_output;

    return true;
}