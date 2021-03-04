#ifndef CLASSIFICATION_NET_H
#define CLASSIFICATION_NET_H

#include <NvInfer.h>
#include <NvUffParser.h>
#include <cuda_runtime_api.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>

#include "../common/include/BatchStream.h"
#include "../common/include/EntropyCalibrator.h"
#include "../common/include/argsParser.h"
#include "../common/include/buffers.h"
#include "../common/include/common.h"
#include "../common/include/logger.h"
#include "../common/include/uff_model.h"

#include "utils/filesystem_include.h"

struct ClassificationNet : UffModel {
   public:
    ClassificationNet(const UffModelParams& params);

    // Run the TensorRT inference engine
    bool infer(const std::vector<cv::Mat>& input_imgs, std::vector<int>& labels, float threshold);

    std::string getClassName(int class_id);

   private:
    // Put input to buffer
    bool processInput(const samplesCommon::BufferManager& buffers, 
                        const std::vector<cv::Mat> &imgs);

    // Process the output
    bool processOutput(const samplesCommon::BufferManager& buffers, std::vector<int> & labels, int n_samples, float threshold);

};

#endif