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

Unet::Unet(const UnetParams& params) {
    mParams = params;

    // If plan file is existed => Load it
    if (!mParams.forceRebuildEngine && mParams.engineFilePath != "" &&
        fs::exists(mParams.engineFilePath)) {
        cout << "Loading TensorRT engine file at: " << mParams.engineFilePath
             << endl;
        loadEngine();

    } else {  // Else, create engine file

        cout << "Creating a new engine file at: "
             << mParams.engineFilePath << endl;
        build();
        if (!saveEngine(mParams.engineFilePath, std::cerr)) {
            cerr << "Error on saving engine at: " << mParams.engineFilePath
             << endl;
            return;
        }
    }

    createContext();
}

bool Unet::loadEngine() {
    std::ifstream engineFile(mParams.engineFilePath, std::ios::binary);
    if (!engineFile) {
        return false;
    }

    engineFile.seekg(0, engineFile.end);
    long int fsize = engineFile.tellg();
    engineFile.seekg(0, engineFile.beg);

    std::vector<char> engineData(fsize);
    engineFile.read(engineData.data(), fsize);

    if (!engineFile) {
        return false;
    }

    SampleUniquePtr<nvinfer1::IRuntime> runtime{
        createInferRuntime(gLogger.getTRTLogger())};

    mEngine = std::shared_ptr<nvinfer1::ICudaEngine>(
        runtime->deserializeCudaEngine(engineData.data(), fsize, nullptr),
        samplesCommon::InferDeleter());
    if (!mEngine) {
        return false;
    }

    return true;
}

bool Unet::saveEngine(const std::string& fileName, std::ostream& err) {
    std::ofstream engineFile(fileName, std::ios::binary);
    if (!engineFile) {
        err << "Cannot open engine file: " << fileName << std::endl;
        return false;
    }

    IHostMemory * serializedEngine = mEngine->serialize();
    if (serializedEngine == nullptr) {
        err << "Engine serialization failed" << std::endl;
        return false;
    }

    engineFile.write(static_cast<char*>(serializedEngine->data()),
                     serializedEngine->size());

    serializedEngine->destroy();
    return !engineFile.fail();
}

// From engine, create context for execution
bool Unet::createContext() {
    // Create RAII buffer manager object
    buffers = std::make_shared<samplesCommon::BufferManager>(mEngine,
                                                             mParams.batchSize);

    context = SampleUniquePtr<nvinfer1::IExecutionContext>(
        mEngine->createExecutionContext());
    if (!context) {
        return false;
    }

    return true;
}

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

            int j = x * inputH + y;

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
            int j = x * inputH + y;
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

//!
//! \brief Creates the network, configures the builder and creates the network
//! engine
//!
//! \details This function creates the MNIST network by parsing the Uff model
//!          and builds the engine that will be used to run MNIST (mEngine)
//!
//! \return Returns true if the engine was created successfully and false
//! otherwise
//!
bool Unet::build() {
    auto builder = SampleUniquePtr<nvinfer1::IBuilder>(
        nvinfer1::createInferBuilder(gLogger.getTRTLogger()));
    if (!builder) {
        return false;
    }
    auto network =
        SampleUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetwork());
    if (!network) {
        return false;
    }
    auto config = SampleUniquePtr<nvinfer1::IBuilderConfig>(
        builder->createBuilderConfig());
    if (!config) {
        return false;
    }
    auto parser = SampleUniquePtr<nvuffparser::IUffParser>(
        nvuffparser::createUffParser());
    if (!parser) {
        return false;
    }
    constructNetwork(parser, network);
    builder->setMaxBatchSize(mParams.batchSize);
    config->setMaxWorkspaceSize(1_GiB);
    config->setFlag(BuilderFlag::kGPU_FALLBACK);
    if (mParams.fp16) {
        config->setFlag(BuilderFlag::kFP16);
    }
    if (mParams.int8) {
        config->setFlag(BuilderFlag::kINT8);
    }

    samplesCommon::enableDLA(builder.get(), config.get(), mParams.dlaCore);

    mEngine = std::shared_ptr<nvinfer1::ICudaEngine>(
        builder->buildEngineWithConfig(*network, *config),
        samplesCommon::InferDeleter());

    if (!mEngine) {
        std::cout << "Error on building engine!" << std::endl;
        exit(1);
        return false;
    }
    assert(network->getNbInputs() == 1);
    mInputDims = network->getInput(0)->getDimensions();
    assert(mInputDims.nbDims == 3);

    // this->createContext();

    return true;
}

//!
//! \brief Uses a Uff parser to create the MNIST Network and marks the output
//! layers
//!
//! \param network Pointer to the network that will be populated with the MNIST
//! network
//!
//! \param builder Pointer to the engine builder
//!
void Unet::constructNetwork(
    SampleUniquePtr<nvuffparser::IUffParser>& parser,
    SampleUniquePtr<nvinfer1::INetworkDefinition>& network) {
    // There should only be one input and one output tensor
    assert(mParams.inputTensorNames.size() == 1);
    assert(mParams.outputTensorNames.size() == 1);

    // Register tensorflow input
    parser->registerInput(mParams.inputTensorNames[0].c_str(),
                          nvinfer1::Dims3(3, mParams.inputW, mParams.inputH),
                          nvuffparser::UffInputOrder::kNCHW);
    parser->registerOutput(mParams.outputTensorNames[0].c_str());

    parser->parse(mParams.uffFilePath.c_str(), *network,
                  nvinfer1::DataType::kFLOAT);

    if (mParams.int8) {
        samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);
    }
}