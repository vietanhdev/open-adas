import re

with open("src/perception/common/onnx_models/onnx-tensorrt/DCNv2.cpp", "r") as f:
    content = f.read()

# getOutputDimensions patch
new_get_out = """nvinfer1::DimsExprs DCNv2Plugin::getOutputDimensions(
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
}"""

content = re.sub(r"nvinfer1::DimsExprs DCNv2Plugin::getOutputDimensions.*?return output;\n}", new_get_out, content, flags=re.DOTALL)

# enqueue patch
new_enqueue = """int DCNv2Plugin::enqueue(
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
}"""

content = re.sub(r"int DCNv2Plugin::enqueue\(.*?return 0;\n}", new_enqueue, content, flags=re.DOTALL)

with open("src/perception/common/onnx_models/onnx-tensorrt/DCNv2.cpp", "w") as f:
    f.write(content)
