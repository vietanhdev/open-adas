cmake_minimum_required(VERSION 3.5)
project(example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast")
include_directories(../include
                    ../onnx-tensorrt)

add_executable(build_tensorrt_engine build_tensorrt_engine.cpp)
target_link_libraries(build_tensorrt_engine ctdet)
