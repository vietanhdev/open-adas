#include "BatchStream.h"
#include "EntropyCalibrator.h"
#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

#include "NvInfer.h"
#include <cuda_runtime_api.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "unet.h"

#include <string>
#include <getopt.h>
#include <vector>

const std::string gName = "TensorRT.unet_segmentation";


void printHelpInfo()
{
    std::cout << "This is help info ..." << std::endl;
}

// Initializes members of the params struct using the command line args
UnetParams initializeParams()
{
    UnetParams params;

    params.inputW = 320;
    params.inputH = 224;
    params.batchSize = 1;
    params.nClasses = 1;

    params.uffFilePath = "../models/full_model_mapillary_lane_resnet18.uff";
    params.engineFilePath = "../models/full_model_mapillary_lane_resnet18.engine";
    params.forceRebuildEngine = false;
    params.inputTensorNames.push_back("data");
    params.outputTensorNames.push_back("sigmoid/Sigmoid");

    return params;
}

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        gLogError << "Invalid arguments" << std::endl;
        printHelpInfo();
        return EXIT_FAILURE;
    }

    auto test = gLogger.defineTest(gName, argc, argv);

    gLogger.reportTestStart(test);

    Unet model(initializeParams());

    gLogInfo << "Building and running a GPU inference engine for Unet" << std::endl;

    // ================= PROCESS IMAGE =======================================
    cv::Mat input_img = cv::imread("../ae60618a-00000000.jpg");
    cv::Mat output_img;

    if (!model.infer(input_img, output_img))
    {
        return gLogger.reportFail(test);
    }

    cv::imshow("Origin", input_img);
    cv::imshow("Output", output_img);
    cv::waitKey(0);
    // ========================================================================

    return gLogger.reportPass(test);
}