cmake_minimum_required(VERSION 3.5)
project(object_detector_trt)
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

add_subdirectory(libs/onnx-tensorrt)

include_directories(${CUDA_INCLUDE_DIRS}
                    ${TENSORRT_INCLUDE_DIR}
                    ${OpenCV_INCLUDE_DIRS}
                    ./include
                    ./libs/onnx-tensorrt
                    ..
                    )

file(GLOB CPP_SRC object_detector/*.cpp)
file(GLOB CU_SRC object_detector/*.cu)
cuda_add_library(object_detector SHARED ${CPP_SRC} ${CU_SRC})
# Use C++ 17
target_compile_features(object_detector PRIVATE cxx_std_17)
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set (CPP_FS_LIB "stdc++fs")
endif()
target_link_libraries(object_detector
        ${TENSORRT_LIBRARY_INFER}
        ${OpenCV_LIBS}
        ${CPP_FS_LIB}
        nvonnxparser
        nvonnxparser_runtime)