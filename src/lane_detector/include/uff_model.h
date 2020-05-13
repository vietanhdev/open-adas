#include <cuda_runtime_api.h>

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>
#include <NvInfer.h>
#include <NvUffParser.h>

#include "common/BatchStream.h"
#include "common/EntropyCalibrator.h"
#include "common/argsParser.h"
#include "common/buffers.h"
#include "common/common.h"
#include "common/logger.h"
#include "common/filesystem_include.h"

struct UffModelParams {
    int inputW;
    int inputH;

    int batchSize{1};
    int nClasses;

    std::string engineFilePath;
    std::string uffFilePath;

    int dlaCore{-1};

    bool int8{false};  //!< Allow runnning the network in Int8 mode.
    bool fp16{false};  //!< Allow running the network in FP16 mode.

    bool forceRebuildEngine{false};

    std::vector<std::string> inputTensorNames;
    std::vector<std::string> outputTensorNames;
};

struct UffModel {

   public:
    template <typename T>
    using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;

    nvinfer1::Dims mInputDims;
    std::shared_ptr<samplesCommon::BufferManager> buffers;

   public:
    UffModel(const UffModelParams& params);

    // Run the TensorRT inference engine
    bool infer(const cv::Mat& input_img, cv::Mat& output_img);

    // Build network from uff file
    bool build();
    //!
    //! \brief Parses a Uff model for MNIST and creates a TensorRT network
    //!
    void constructNetwork(
        SampleUniquePtr<nvuffparser::IUffParser>& parser,
        SampleUniquePtr<nvinfer1::INetworkDefinition>& network);

   public:
    UffModelParams mParams;

    std::shared_ptr<nvinfer1::ICudaEngine> mEngine;

    SampleUniquePtr<nvinfer1::IExecutionContext> context;

    // Read engine file and creates a TensorRT network
    bool loadEngine();

    // Save engine to file
    bool saveEngine(const std::string& fileName,
                std::ostream& err);

    // Create execution context
    bool createContext();

};