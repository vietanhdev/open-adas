//
// Created by cao on 19-10-26.
//

#include <assert.h>
#include <fstream>
#include "ctdetNet.h"
#include "ctdetLayer.h"
// #include "entroyCalibrator.h" // Removed for TRT 10 port

static CTDetLogger gLogger;

namespace ctdet
{

    ctdetNet::ctdetNet(const std::string &onnxFile, const std::string &calibFile,
            ctdet::RUN_MODE mode):forwardFace(false),mContext(nullptr),mEngine(nullptr),mRunTime(nullptr),
                                  runMode(mode),runIters(0)
    {

        const int maxBatchSize = 1;
        nvinfer1::IHostMemory *modelStream{nullptr};
        int verbosity = (int) nvinfer1::ILogger::Severity::kWARNING;
        nvinfer1::IBuilder* builder = nvinfer1::createInferBuilder(gLogger);
        
        const auto explicitBatch = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
        nvinfer1::INetworkDefinition* network = builder->createNetworkV2(explicitBatch);

        auto parser = nvonnxparser::createParser(*network, gLogger);
        if (!parser->parseFromFile(onnxFile.c_str(), verbosity))
        {
            std::string msg("failed to parse onnx file");
            gLogger.log(nvinfer1::ILogger::Severity::kERROR, msg.c_str());
            exit(EXIT_FAILURE);
        }

        nvinfer1::IBuilderConfig* config = builder->createBuilderConfig();
        config->setMemoryPoolLimit(nvinfer1::MemoryPoolType::kWORKSPACE, 1ULL << 30); // 1G

        if (runMode == RUN_MODE::INT8)
        {
            std::cout << "setInt8Mode (Not supported in TRT 10 port currently)" << std::endl;
        }
        else if (runMode == RUN_MODE::FLOAT16)
        {
            std::cout << "setFp16Mode" << std::endl;
            config->setFlag(nvinfer1::BuilderFlag::kFP16);
        }

        std::cout << "Begin building engine..." << std::endl;
        modelStream = builder->buildSerializedNetwork(*network, *config);
        if (!modelStream){
            std::string error_message ="Unable to create engine";
            gLogger.log(nvinfer1::ILogger::Severity::kERROR, error_message.c_str());
            exit(-1);
        }
        std::cout << "End building engine..." << std::endl;

        mRunTime = nvinfer1::createInferRuntime(gLogger);
        assert(mRunTime != nullptr);
        mEngine = mRunTime->deserializeCudaEngine(modelStream->data(), modelStream->size());
        assert(mEngine != nullptr);

        delete modelStream;
        delete network;
        delete config;
        delete builder;
        delete parser;
        
        InitEngine();

    }

    ctdetNet::ctdetNet(const std::string &engineFile)
            :forwardFace(false),mContext(nullptr),mEngine(nullptr),mRunTime(nullptr),runMode(RUN_MODE::FLOAT32),runIters(0)
    {
        using namespace std;
        fstream file;

        file.open(engineFile,ios::binary | ios::in);
        if(!file.is_open())
        {
            cout << "read engine file" << engineFile <<" failed" << endl;
            return;
        }
        file.seekg(0, ios::end);
        int length = file.tellg();
        file.seekg(0, ios::beg);
        std::unique_ptr<char[]> data(new char[length]);
        file.read(data.get(), length);

        file.close();

        std::cout << "deserializing" << std::endl;
        mRunTime = nvinfer1::createInferRuntime(gLogger);
        assert(mRunTime != nullptr);
        mEngine = mRunTime->deserializeCudaEngine(data.get(), length);
        assert(mEngine != nullptr);
        InitEngine();
    }

    void ctdetNet::InitEngine() {
        const int maxBatchSize = 1;
        mContext = mEngine->createExecutionContext();
        assert(mContext != nullptr);
        mContext->setProfiler(&mProfiler);
        
        int32_t nbTensors = mEngine->getNbIOTensors();
        
        mIOTensorNames.clear();
        std::vector<std::string> temp_outputs;
        for (int i = 0; i < nbTensors; ++i) {
            const char* name = mEngine->getIOTensorName(i);
            if (mEngine->getTensorIOMode(name) == nvinfer1::TensorIOMode::kINPUT) {
                mIOTensorNames.insert(mIOTensorNames.begin(), name);
            } else {
                temp_outputs.push_back(name);
            }
        }
        for (const auto& name : temp_outputs) {
            mIOTensorNames.push_back(name);
        }
        
        int nbBindings = mIOTensorNames.size();

        if (nbBindings > 4) forwardFace= true;

        mCudaBuffers.resize(nbBindings);
        mBindBufferSizes.resize(nbBindings);
        int64_t totalSize = 0;
        for (int i = 0; i < nbBindings; ++i)
        {
            const char* name = mIOTensorNames[i].c_str();
            nvinfer1::Dims dims = mEngine->getTensorShape(name);
            nvinfer1::DataType dtype = mEngine->getTensorDataType(name);
            totalSize = volume(dims) * maxBatchSize * getElementSize(dtype);
            mBindBufferSizes[i] = totalSize;
            mCudaBuffers[i] = safeCudaMalloc(totalSize);
        }
        outputBufferSize = mBindBufferSizes[1] * 6 ;
        cudaOutputBuffer = safeCudaMalloc(outputBufferSize);
        CUDA_CHECK(cudaStreamCreate(&mCudaStream));
    }

    void ctdetNet::doInference(const void *inputData, void *outputData)
    {
        int inputIndex = 0 ;
        CUDA_CHECK(cudaMemcpyAsync(mCudaBuffers[inputIndex], inputData, mBindBufferSizes[inputIndex], cudaMemcpyHostToDevice, mCudaStream));
        
        for (size_t i = 0; i < mIOTensorNames.size(); ++i) {
            mContext->setTensorAddress(mIOTensorNames[i].c_str(), mCudaBuffers[i]);
        }
        mContext->enqueueV3(mCudaStream);
        
        CUDA_CHECK(cudaMemset(cudaOutputBuffer, 0, sizeof(float)));
        if (forwardFace){
            CTfaceforward_gpu(static_cast<const float *>(mCudaBuffers[1]),static_cast<const float *>(mCudaBuffers[2]),
                              static_cast<const float *>(mCudaBuffers[3]),static_cast<const float *>(mCudaBuffers[4]),static_cast<float *>(cudaOutputBuffer),
                              input_w/4,input_h/4,classNum,kernelSize,visThresh);
        } else{
            CTdetforward_gpu(static_cast<const float *>(mCudaBuffers[1]),static_cast<const float *>(mCudaBuffers[2]),
                         static_cast<const float *>(mCudaBuffers[3]),static_cast<float *>(cudaOutputBuffer),
                             input_w/4,input_h/4,classNum,kernelSize,visThresh);
        }

        CUDA_CHECK(cudaMemcpyAsync(outputData, cudaOutputBuffer, outputBufferSize, cudaMemcpyDeviceToHost, mCudaStream));

        runIters++ ;
    }
    void ctdetNet::saveEngine(const std::string &fileName)
    {
        if(mEngine)
        {
            nvinfer1::IHostMemory* data = mEngine->serialize();
            std::ofstream file;
            file.open(fileName,std::ios::binary | std::ios::out);
            if(!file.is_open())
            {
                std::cout << "read create engine file" << fileName <<" failed" << std::endl;
                return;
            }
            file.write((const char*)data->data(), data->size());
            file.close();
            delete data;
        }

    }
}