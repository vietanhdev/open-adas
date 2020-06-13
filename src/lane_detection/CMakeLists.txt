cmake_minimum_required(VERSION 3.5)
project(lane_detector)

find_package(CUDA REQUIRED)

find_path(TENSORRT_INCLUDE_DIR NvInfer.h
        HINTS ${TENSORRT_ROOT} ${CUDA_TOOLKIT_ROOT_DIR}
        PATH_SUFFIXES include/)
message(STATUS "Found TensorRT headers at ${TENSORRT_INCLUDE_DIR}")

find_library(TENSORRT_LIBRARY_INFER nvinfer
        HINTS ${TENSORRT_ROOT} ${TENSORRT_BUILD} ${CUDA_TOOLKIT_ROOT_DIR}
        PATH_SUFFIXES lib lib64 lib/x64)
message(STATUS "Found TensorRT libs ${TENSORRT_LIBRARY_INFER}")

find_package(OpenCV REQUIRED)
link_directories(${OpenCV_LIBRARIES_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast")
list(APPEND CUDA_NVCC_FLAGS "-D_FORCE_INLINES -Xcompiler -fPIC")


message(${TENSORRT_INCLUDE_DIR})
message(STATUS ${TENSORRT_ROOT})
include_directories(${CUDA_INCLUDE_DIRS}
                    ${TENSORRT_INCLUDE_DIR}
                    ${OpenCV_INCLUDE_DIRS}
                    ../common/uff_models/common/include
                    ../common/uff_models/unet
                    ../common/utils/include
                    ../configs
                    )

file(GLOB UFF_MODEL_CPP ../common/uff_models/common/*.cpp)
file(GLOB UNET_CPP ../common/uff_models/unet/*.cpp)

cuda_add_library(lane_detector lane_detector.cpp ${UFF_MODEL_CPP} ${UNET_CPP})

# Use C++ 17
target_compile_features(lane_detector PRIVATE cxx_std_17)
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set (CPP_FS_LIB "stdc++fs")
endif()

target_link_libraries(lane_detector
        ${TENSORRT_LIBRARY_INFER}
        ${OpenCV_LIBS}
        ${CPP_FS_LIB}
        nvparsers
        nvonnxparser
        nvinfer_plugin
)


cuda_add_executable(test_lane_detector
        test_lane_detector.cpp
        ../common/utils/timer.cpp
        )
target_link_libraries(test_lane_detector
        lane_detector
)
