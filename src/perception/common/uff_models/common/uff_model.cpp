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
#include "uff_model.h"

UffModel::UffModel(const UffModelParams& params) {
    mParams = params;

    // If plan file is existed => Load it
    if (!mParams.forceRebuildEngine && mParams.engineFilePath != "" &&
        fs::exists(mParams.engineFilePath)) {
        cout << "Loading TensorRT engine file at: " << mParams.engineFilePath
             << endl;
        loadEngine();

    } else {  // Else, create engine file

        cout << "Creating a new engine file at: " << mParams.engineFilePath
             << endl;
        build();
        if (!saveEngine(mParams.engineFilePath, std::cerr)) {
            cerr << "Error on saving engine at: " << mParams.engineFilePath
                 << endl;
            return;
        }
    }

    if (mParams.classListFile != "") {
        if (readClassListFile(mParams.classListFile, mParams.classes) != 0) {
            cerr << "Error on loading class list: " << mParams.classListFile
                 << endl;
            return;
        }
        cout << mParams.nClasses << endl;
        mParams.nClasses = mParams.classes.size();
    }

    createContext();
}

bool UffModel::loadEngine() {
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

bool UffModel::saveEngine(const std::string& fileName, std::ostream& err) {
    std::ofstream engineFile(fileName, std::ios::binary);
    if (!engineFile) {
        err << "Cannot open engine file: " << fileName << std::endl;
        return false;
    }

    IHostMemory* serializedEngine = mEngine->serialize();
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
bool UffModel::createContext() {
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
bool UffModel::build() {
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
void UffModel::constructNetwork(
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

// Read class list from file
// Return 0 if success, otherwise return a positive number
int UffModel::readClassListFile(const std::string& class_list_file,
                                std::vector<ObjectClass>& classes) {
    // Read data file
    std::ifstream data_file(class_list_file);
    if (!data_file) {
        return 1;
    }

    classes.clear();

    std::string line;
    while (std::getline(data_file, line)) {
        trim(line);

        // Split string
        std::vector<std::string> splited;
        std::istringstream iss(line);
        for (std::string s; iss >> s;) splited.push_back(s);

        // Read class data
        if (splited.size() >= 2) {
            int class_id = std::stoi(splited[0]);
            std::string class_name = splited[1];
            classes.push_back(ObjectClass(class_id, class_name));
        }
    }

    return 0;
}

std::string& UffModel::ltrim(std::string& str, const std::string& chars) {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string& UffModel::rtrim(std::string& str, const std::string& chars) {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string& UffModel::trim(std::string& str, const std::string& chars) {
    return ltrim(rtrim(str, chars), chars);
}