//
// Created by cao on 19-12-16.
//

#ifndef CTDET_TRT_ENTROYCALIBRATOR_H
#define CTDET_TRT_ENTROYCALIBRATOR_H

#include "NvInfer.h"
#include <vector>
#include <string>
namespace nvinfer1 {
    class int8EntroyCalibrator : public nvinfer1::IInt8EntropyCalibrator {
    public:
        int8EntroyCalibrator(const int &bacthSize,
                             const std::string &imgPath,
                             const std::string &calibTablePath);

        virtual ~int8EntroyCalibrator();

        int getBatchSize() const noexcept override { return batchSize; }

        bool getBatch(void *bindings[], const char *names[], int nbBindings) noexcept override;

        const void *readCalibrationCache(std::size_t &length) noexcept override;

        void writeCalibrationCache(const void *ptr, std::size_t length) noexcept override;

    private:

        bool forwardFace;

        int batchSize;
        size_t inputCount;
        size_t imageIndex;

        std::string calibTablePath;
        std::vector<std::string> imgPaths;

        float *batchData{ nullptr };
        void  *deviceInput{ nullptr };



        bool readCache;
        std::vector<char> calibrationCache;
    };
}


#endif //CTDET_TRT_ENTROYCALIBRATOR_H
