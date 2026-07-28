#include "DCNv2.hpp"
#include "dcn_v2_im2col_cuda.h"
#include <stdexcept>
#include <cstring>
#include <cassert>

#define CHECK_CUDA(call) do {    \
  cudaError_t status = call; \
  if( status != cudaSuccess ) { \
    return status; \
  } \
} while(0)

template <typename T>
void writeToBuffer(char*& buffer, const T& val) {
    std::memcpy(buffer, &val, sizeof(T));
    buffer += sizeof(T);
}

template <typename T>
void writeVectorToBuffer(char*& buffer, const std::vector<T>& vec) {
    size_t size = vec.size();
    writeToBuffer(buffer, size);
    if (size > 0) {
        std::memcpy(buffer, vec.data(), size * sizeof(T));
    }
    buffer += size * sizeof(T);
}

template <typename T>
void readFromBuffer(const char*& buffer, T& val) {
    std::memcpy(&val, buffer, sizeof(T));
    buffer += sizeof(T);
}

template <typename T>
void readVectorFromBuffer(const char*& buffer, std::vector<T>& vec) {
    size_t size;
    readFromBuffer(buffer, size);
    vec.resize(size);
    if (size > 0) {
        std::memcpy(vec.data(), buffer, size * sizeof(T));
    }
    buffer += size * sizeof(T);
}

cublasHandle_t blas_handle()
{
    static int init[16] = {0};
    static cublasHandle_t handle[16];
    int n = 0;
    cudaError_t status = cudaGetDevice(&n);
    if(!init[n]) {
        cublasCreate(&handle[n]);
        init[n] = 1;
    }
    return handle[n];
}

DCNv2Plugin::DCNv2Plugin(int in_channel,
                         int out_channel,
                         int kernel_H,
                         int kernel_W,
                         int deformable_group,
                         int dilation,
                         int groups,
                         int padding,
                         int stride,
                         nvinfer1::Weights const &weight, nvinfer1::Weights const &bias):_in_channel(in_channel),
                        _out_channel(out_channel),_kernel_H(kernel_H),_kernel_W(kernel_W),_deformable_group(deformable_group),
                         _dilation(dilation),_groups(groups),_padding(padding),_stride(stride),_initialized(false),_d_weight(nullptr),_d_bias(nullptr),_d_ones(nullptr),_d_columns(nullptr){

    if (weight.type == nvinfer1::DataType::kFLOAT)
    {
        _h_weight.assign((float*)weight.values,(float*)weight.values+weight.count);
    } else { throw std::runtime_error("Unsupported  weight dtype");}

    if (bias.type == nvinfer1::DataType::kFLOAT)
    {
        _h_bias.assign((float*)bias.values,(float*)bias.values+bias.count);
    } else { throw std::runtime_error("Unsupported  bias dtype");}
}

DCNv2Plugin::DCNv2Plugin(void const* serialData, size_t serialLength) : _initialized(false), _d_weight(nullptr), _d_bias(nullptr), _d_ones(nullptr), _d_columns(nullptr) {
    const char* d = static_cast<const char*>(serialData);
    readFromBuffer(d, _in_channel);
    readFromBuffer(d, _out_channel);
    readFromBuffer(d, _kernel_H);
    readFromBuffer(d, _kernel_W);
    readFromBuffer(d, _deformable_group);
    readFromBuffer(d, _dilation);
    readFromBuffer(d, _groups);
    readFromBuffer(d, _padding);
    readFromBuffer(d, _stride);
    readVectorFromBuffer(d, _h_weight);
    readVectorFromBuffer(d, _h_bias);
}

size_t DCNv2Plugin::getSerializationSize() const noexcept {
    return sizeof(int) * 9 + sizeof(size_t) * 2 + _h_weight.size() * sizeof(float) + _h_bias.size() * sizeof(float);
}

void DCNv2Plugin::serialize(void *buffer) const noexcept {
    char* d = static_cast<char*>(buffer);
    writeToBuffer(d, _in_channel);
    writeToBuffer(d, _out_channel);
    writeToBuffer(d, _kernel_H);
    writeToBuffer(d, _kernel_W);
    writeToBuffer(d, _deformable_group);
    writeToBuffer(d, _dilation);
    writeToBuffer(d, _groups);
    writeToBuffer(d, _padding);
    writeToBuffer(d, _stride);
    writeVectorToBuffer(d, _h_weight);
    writeVectorToBuffer(d, _h_bias);
}

nvinfer1::IPluginV2DynamicExt* DCNv2Plugin::clone() const noexcept {
    nvinfer1::Weights weight{nvinfer1::DataType::kFLOAT, _h_weight.data(), (int64_t)_h_weight.size()};
    nvinfer1::Weights bias{nvinfer1::DataType::kFLOAT, _h_bias.data(), (int64_t)_h_bias.size()};
    auto* plugin = new DCNv2Plugin(_in_channel, _out_channel, _kernel_H, _kernel_W, _deformable_group, _dilation, _groups, _padding, _stride, weight, bias);
    plugin->setPluginNamespace(mNamespace.c_str());
    return plugin;
}

int DCNv2Plugin::initialize() noexcept {
    if(_initialized) return 0;
    size_t weight_size = _h_weight.size() * sizeof(float);
    size_t bias_size = _h_bias.size() * sizeof(float);
    CHECK_CUDA(cudaMalloc((void**)&_d_weight, weight_size));
    CHECK_CUDA(cudaMalloc((void**)&_d_bias, bias_size));
    CHECK_CUDA(cudaMemcpy(_d_weight, _h_weight.data(), weight_size, cudaMemcpyHostToDevice));
    CHECK_CUDA(cudaMemcpy(_d_bias, _h_bias.data(), bias_size, cudaMemcpyHostToDevice));
    _initialized = true;
    return 0;
}
void DCNv2Plugin::terminate() noexcept {
    if (!_initialized) {
        return;
    }
    if (_d_columns) cudaFree(_d_columns);
    if (_d_bias) cudaFree(_d_bias);
    if (_d_weight) cudaFree(_d_weight);
    if (_d_ones) cudaFree(_d_ones);
    _initialized = false;
    _d_columns = nullptr;
    _d_bias = nullptr;
    _d_weight = nullptr;
    _d_ones = nullptr;
}

DCNv2Plugin::~DCNv2Plugin() {
    terminate();
}

nvinfer1::DataType DCNv2Plugin::getOutputDataType(int index, const nvinfer1::DataType* inputTypes, int nbInputs) const noexcept {
    return nvinfer1::DataType::kFLOAT;
}

nvinfer1::DimsExprs DCNv2Plugin::getOutputDimensions(
    int outputIndex, const nvinfer1::DimsExprs* inputs, int nbInputs, nvinfer1::IExprBuilder& exprBuilder) noexcept {
    nvinfer1::DimsExprs output(inputs[0]);
    output.d[0] = inputs[0].d[0];
    
    if (nbInputs > 3) {
        output.d[1] = inputs[3].d[0];
    } else {
        output.d[1] = exprBuilder.constant(_out_channel);
    }
    
    auto const* h = inputs[0].d[2];
    auto const* w = inputs[0].d[3];
    
    auto const* kernel_H = (nbInputs > 3) ? inputs[3].d[2] : exprBuilder.constant(_kernel_H);
    auto const* kernel_W = (nbInputs > 3) ? inputs[3].d[3] : exprBuilder.constant(_kernel_W);

    auto* h_padding = exprBuilder.constant(2 * _padding);
    auto* h_kernel = exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *exprBuilder.operation(nvinfer1::DimensionOperation::kPROD, *exprBuilder.constant(_dilation), *exprBuilder.operation(nvinfer1::DimensionOperation::kSUB, *kernel_H, *exprBuilder.constant(1))), *exprBuilder.constant(1));
    auto* h_stride = exprBuilder.constant(_stride);
    auto* h_out = exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *exprBuilder.operation(nvinfer1::DimensionOperation::kFLOOR_DIV, *exprBuilder.operation(nvinfer1::DimensionOperation::kSUB, *exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *h, *h_padding), *h_kernel), *h_stride), *exprBuilder.constant(1));
    output.d[2] = h_out;
    
    auto* w_padding = exprBuilder.constant(2 * _padding);
    auto* w_kernel = exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *exprBuilder.operation(nvinfer1::DimensionOperation::kPROD, *exprBuilder.constant(_dilation), *exprBuilder.operation(nvinfer1::DimensionOperation::kSUB, *kernel_W, *exprBuilder.constant(1))), *exprBuilder.constant(1));
    auto* w_stride = exprBuilder.constant(_stride);
    auto* w_out = exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *exprBuilder.operation(nvinfer1::DimensionOperation::kFLOOR_DIV, *exprBuilder.operation(nvinfer1::DimensionOperation::kSUB, *exprBuilder.operation(nvinfer1::DimensionOperation::kSUM, *w, *w_padding), *w_kernel), *w_stride), *exprBuilder.constant(1));
    output.d[3] = w_out;
    
    return output;
}

bool DCNv2Plugin::supportsFormatCombination(
    int pos, const nvinfer1::PluginTensorDesc* inOut, int nbInputs, int nbOutputs) noexcept {
    return inOut[pos].type == nvinfer1::DataType::kFLOAT && inOut[pos].format == nvinfer1::PluginFormat::kLINEAR;
}

void DCNv2Plugin::configurePlugin(
    const nvinfer1::DynamicPluginTensorDesc* in, int nbInputs, const nvinfer1::DynamicPluginTensorDesc* out, int nbOutputs) noexcept {
}

size_t DCNv2Plugin::getWorkspaceSize(
    const nvinfer1::PluginTensorDesc* inputs, int nbInputs, const nvinfer1::PluginTensorDesc* outputs, int nbOutputs) const noexcept {
    return 0;
}

int DCNv2Plugin::enqueue(
    const nvinfer1::PluginTensorDesc* inputDesc, const nvinfer1::PluginTensorDesc* outputDesc,
    const void* const* inputs, void* const* outputs, void* workspace, cudaStream_t stream) noexcept {
    
    int batchSize = inputDesc[0].dims.d[0];
    int h = inputDesc[0].dims.d[2];
    int w = inputDesc[0].dims.d[3];
    
    int out_channel = _out_channel;
    int in_channel = _in_channel;
    int kernel_H = _kernel_H;
    int kernel_W = _kernel_W;
    const float* weight_ptr = _d_weight;
    const float* bias_ptr = _d_bias;

    // Check how many inputs were provided. In TRT 10 ONNX parser, W and B are passed as inputs[3] and inputs[4].
    // Count nbInputs by checking inputDesc (wait, enqueue doesn't have nbInputs, but we can assume if not initialized, we use inputs[3])
    // Actually, TRT passes exactly what the ONNX parser gives.
    // DCNv2 takes 5 inputs in ONNX.
    bool has_wb_inputs = (weight_ptr == nullptr); // If _d_weight is null, we assume they are passed as inputs.
    
    if (has_wb_inputs) {
        out_channel = inputDesc[3].dims.d[0];
        in_channel = inputDesc[3].dims.d[1] * _groups;
        kernel_H = inputDesc[3].dims.d[2];
        kernel_W = inputDesc[3].dims.d[3];
        weight_ptr = static_cast<const float*>(inputs[3]);
        bias_ptr = static_cast<const float*>(inputs[4]);
    }
    
    int height_out = (h + 2 * _padding - (_dilation * (kernel_H - 1) + 1)) / _stride + 1;
    int width_out = (w + 2 * _padding - (_dilation * (kernel_W - 1) + 1)) / _stride + 1;
    
    size_t ones_size = height_out * width_out * sizeof(float);
    size_t columns_size = in_channel * kernel_H * kernel_W * ones_size;
    
    if (!_d_ones) {
        float *ones_cpu = new float[height_out * width_out];
        for (int i = 0; i < height_out * width_out; i++) ones_cpu[i] = 1.0;
        cudaMalloc((void**)&_d_ones, ones_size);
        cudaMemcpy(_d_ones, ones_cpu, ones_size, cudaMemcpyHostToDevice);
        delete[] ones_cpu;
        
        cudaMalloc((void**)&_d_columns, columns_size);
    }
    
    cublasHandle_t handle = blas_handle();
    cublasSetStream(handle, stream);
    
    float alpha = 1.0;
    float beta = 0.0;

    for (int b = 0; b < batchSize; ++b) {
        const float* input = static_cast<const float *>(inputs[0]) + b * in_channel * h * w;
        const float* offset = static_cast<const float *>(inputs[1]) + b * 2 * _deformable_group * kernel_H * kernel_W * h * w;
        const float* mask = static_cast<const float *>(inputs[2]) + b * _deformable_group * kernel_H * kernel_W * h * w;
        float * output = static_cast<float *>(outputs[0]) + b * out_channel * height_out * width_out;

        int m = out_channel;
        int n = height_out * width_out;
        int k = 1;
        alpha = 1.0;
        beta = 0.0;
        
        cublasSgemm(handle,
                    CUBLAS_OP_T, CUBLAS_OP_N,
                    n, m, k,&alpha,
                    _d_ones, k,
                    bias_ptr, k,&beta,
                    output, n);

        modulated_deformable_im2col_cuda(stream,input,offset,mask,
                                         1, in_channel, h, w,
                                         height_out, width_out, kernel_H, kernel_W,
                                         _padding, _padding, _stride, _stride, _dilation, _dilation,
                                         _deformable_group, _d_columns);
        m = out_channel;
        n = height_out * width_out;
        k = in_channel * kernel_H * kernel_W;
        alpha = 1.0;
        beta = 1.0;

        cublasSgemm(handle,
                    CUBLAS_OP_N, CUBLAS_OP_N,
                    n, m, k,&alpha,
                    _d_columns, n,
                    weight_ptr, k,
                    &beta,
                    output, n);
    }
    return 0;
}

std::vector<nvinfer1::PluginField> DCNv2PluginCreator::mPluginAttributes;
nvinfer1::PluginFieldCollection DCNv2PluginCreator::mFC{};

DCNv2PluginCreator::DCNv2PluginCreator() {
    mPluginAttributes.clear();
    mFC.nbFields = mPluginAttributes.size();
    mFC.fields = mPluginAttributes.data();
}

const char* DCNv2PluginCreator::getPluginName() const noexcept {
    return "DCNv2";
}

const char* DCNv2PluginCreator::getPluginVersion() const noexcept {
    return "1";
}

const nvinfer1::PluginFieldCollection* DCNv2PluginCreator::getFieldNames() noexcept {
    return &mFC;
}

nvinfer1::IPluginV2* DCNv2PluginCreator::createPlugin(const char* name, const nvinfer1::PluginFieldCollection* fc) noexcept {
    int in_channel = 0, out_channel = 0, kernel_H = 0, kernel_W = 0, deformable_group = 1, dilation = 1, groups = 1, padding = 1, stride = 1;
    nvinfer1::Weights weight{nvinfer1::DataType::kFLOAT, nullptr, 0};
    nvinfer1::Weights bias{nvinfer1::DataType::kFLOAT, nullptr, 0};

    for (int i = 0; i < fc->nbFields; ++i) {
        std::string field_name(fc->fields[i].name);
        if (field_name.compare("in_channel") == 0) in_channel = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("out_channel") == 0) out_channel = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("kernel_H") == 0) kernel_H = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("kernel_W") == 0) kernel_W = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("deformable_group") == 0) deformable_group = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("dilation") == 0 || field_name.compare("dilations") == 0) dilation = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("groups") == 0) groups = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("padding") == 0 || field_name.compare("pads") == 0) padding = static_cast<const int*>(fc->fields[i].data)[0];
        if (field_name.compare("stride") == 0 || field_name.compare("strides") == 0) stride = static_cast<const int*>(fc->fields[i].data)[0];
        // Note: the original vendored ONNX plugin manually parsed weight/bias. TRT10 system ONNX parser passes them like this.
        if (field_name.compare("W") == 0) { 
            weight.values = fc->fields[i].data;
            weight.count = fc->fields[i].length;
            weight.type = nvinfer1::DataType::kFLOAT;
        }
        if (field_name.compare("B") == 0) { 
            bias.values = fc->fields[i].data;
            bias.count = fc->fields[i].length;
            bias.type = nvinfer1::DataType::kFLOAT;
        }
    }
    
    DCNv2Plugin* obj = new DCNv2Plugin(in_channel, out_channel, kernel_H, kernel_W, deformable_group, dilation, groups, padding, stride, weight, bias);
    obj->setPluginNamespace(mNamespace.c_str());
    return obj;
}

nvinfer1::IPluginV2* DCNv2PluginCreator::deserializePlugin(const char* name, const void* serialData, size_t serialLength) noexcept {
    DCNv2Plugin* obj = new DCNv2Plugin(serialData, serialLength);
    obj->setPluginNamespace(mNamespace.c_str());
    return obj;
}

void DCNv2PluginCreator::setPluginNamespace(const char* pluginNamespace) noexcept {
    mNamespace = pluginNamespace;
}

const char* DCNv2PluginCreator::getPluginNamespace() const noexcept {
    return mNamespace.c_str();
}

REGISTER_TENSORRT_PLUGIN(DCNv2PluginCreator);
