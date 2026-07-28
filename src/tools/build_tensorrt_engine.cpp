#include <iostream>
#include <string>
#include "perception/common/onnx_models/include/ctdetNet.h"
#include <unistd.h>

int main(int argc, char** argv) {
    std::string onnxFile = "";
    std::string engineFile = "";
    int mode = 0; // 0 for FLOAT32, 1 for FLOAT16, 2 for INT8
    
    int opt;
    while ((opt = getopt(argc, argv, "i:o:m:")) != -1) {
        switch (opt) {
            case 'i':
                onnxFile = optarg;
                break;
            case 'o':
                engineFile = optarg;
                break;
            case 'm':
                mode = std::atoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -i <onnx_file> -o <engine_file> [-m <mode:0=fp32,1=fp16>]" << std::endl;
                return 1;
        }
    }
    
    if (onnxFile.empty() || engineFile.empty()) {
        std::cerr << "Usage: " << argv[0] << " -i <onnx_file> -o <engine_file> [-m <mode:0=fp32,1=fp16>]" << std::endl;
        return 1;
    }
    
    std::cout << "Building engine from " << onnxFile << " to " << engineFile << " with mode " << mode << std::endl;
    ctdet::RUN_MODE run_mode = ctdet::RUN_MODE::FLOAT32;
    if (mode == 1) run_mode = ctdet::RUN_MODE::FLOAT16;
    else if (mode == 2) run_mode = ctdet::RUN_MODE::INT8;
    
    ctdet::ctdetNet net(onnxFile, "", run_mode);
    net.saveEngine(engineFile);
    std::cout << "Engine saved successfully." << std::endl;
    return 0;
}
