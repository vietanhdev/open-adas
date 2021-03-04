#ifndef UFF_MODEL_H
#define UFF_MODEL_H

#include <cuda_runtime_api.h>
#include <vector>
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

#include "BatchStream.h"
#include "EntropyCalibrator.h"
#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"
#include "filesystem_include.h"
#include "object_class.h"

struct UffModelParams {
    int inputW;
    int inputH;

    int batchSize{1};
    int nClasses{-1};
    std::vector<ObjectClass> classes;
    std::string classListFile;

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

    // Read class list from file
    // Return 0 if success, otherwise return a positive number
    int readClassListFile(const std::string & class_list_file, std::vector<ObjectClass> &classes);

   private:
    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");

};

#endif