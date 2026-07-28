#ifndef ONNX2TRT_DCNV2_H
#define ONNX2TRT_DCNV2_H

#pragma once

#include "NvInfer.h"
#include <cudnn.h>
#include <vector>
#include <string>
#include <cublas_v2.h>
#include <cuda.h>

class DCNv2Plugin final : public nvinfer1::IPluginV2DynamicExt {
    int _in_channel;
    int _out_channel;
    int _kernel_H;
    int _kernel_W;
    int _deformable_group;
    int _dilation;
    int _groups; // not use
    int _padding;
    int _stride;
    std::vector<float> _h_weight;
    std::vector<float> _h_bias;
    float* _d_weight;
    float* _d_bias;
    float* _d_ones;
    float *_d_columns;

    bool _initialized;
    std::string mNamespace;

public:
    DCNv2Plugin(int in_channel,
                int out_channel,
                int kernel_H,
                int kernel_W,
                int deformable_group,
                int dilation,
                int groups,
                int padding,
                int stride,
                nvinfer1::Weights const& weight,
                nvinfer1::Weights const& bias);

    DCNv2Plugin(void const* serialData, size_t serialLength);
    ~DCNv2Plugin() override;

    // IPluginV2 methods
    const char* getPluginType() const noexcept override { return "DCNv2"; }
    const char* getPluginVersion() const noexcept override { return "1"; }
    int getNbOutputs() const noexcept override { return 1; }
    int initialize() noexcept override;
    void terminate() noexcept override;
    size_t getSerializationSize() const noexcept override;
    void serialize(void* buffer) const noexcept override;
    void destroy() noexcept override { delete this; }
    void setPluginNamespace(const char* pluginNamespace) noexcept override { mNamespace = pluginNamespace; }
    const char* getPluginNamespace() const noexcept override { return mNamespace.c_str(); }

    // IPluginV2Ext methods
    nvinfer1::DataType getOutputDataType(int index, const nvinfer1::DataType* inputTypes, int nbInputs) const noexcept override;

    // IPluginV2DynamicExt methods
    nvinfer1::IPluginV2DynamicExt* clone() const noexcept override;
    nvinfer1::DimsExprs getOutputDimensions(
        int outputIndex, const nvinfer1::DimsExprs* inputs, int nbInputs, nvinfer1::IExprBuilder& exprBuilder) noexcept override;
    bool supportsFormatCombination(
        int pos, const nvinfer1::PluginTensorDesc* inOut, int nbInputs, int nbOutputs) noexcept override;
    void configurePlugin(
        const nvinfer1::DynamicPluginTensorDesc* in, int nbInputs, const nvinfer1::DynamicPluginTensorDesc* out, int nbOutputs) noexcept override;
    size_t getWorkspaceSize(
        const nvinfer1::PluginTensorDesc* inputs, int nbInputs, const nvinfer1::PluginTensorDesc* outputs, int nbOutputs) const noexcept override;
    int enqueue(
        const nvinfer1::PluginTensorDesc* inputDesc, const nvinfer1::PluginTensorDesc* outputDesc,
        const void* const* inputs, void* const* outputs, void* workspace, cudaStream_t stream) noexcept override;
};

class DCNv2PluginCreator : public nvinfer1::IPluginCreator {
public:
    DCNv2PluginCreator();
    const char* getPluginName() const noexcept override;
    const char* getPluginVersion() const noexcept override;
    const nvinfer1::PluginFieldCollection* getFieldNames() noexcept override;
    nvinfer1::IPluginV2* createPlugin(const char* name, const nvinfer1::PluginFieldCollection* fc) noexcept override;
    nvinfer1::IPluginV2* deserializePlugin(const char* name, const void* serialData, size_t serialLength) noexcept override;
    void setPluginNamespace(const char* pluginNamespace) noexcept override;
    const char* getPluginNamespace() const noexcept override;

private:
    static nvinfer1::PluginFieldCollection mFC;
    static std::vector<nvinfer1::PluginField> mPluginAttributes;
    std::string mNamespace;
};

#endif //ONNX2TRT_DCNV2_H
