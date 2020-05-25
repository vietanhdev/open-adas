cmake_minimum_required(VERSION 3.5)
project(car_status)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -fPIC")

include_directories(
    ../lane_detection
    ../object_detection
    ../object_detection/traffic_sign_classifier
    ../common/uff_models/common/include
    ../common/uff_models/unet
    ../common/uff_models/classification_net
    ../common/utils/include
    ../common/onnx_models/include
    ../common/onnx_models/onnx-tensorrt
    ../configs
)

add_subdirectory(libs/NemaTode)     
include_directories(include)

add_library(car_status car_status.cpp)
target_link_libraries(car_status NemaTode)

add_library(car_gps car_gps_reader.cpp)
target_link_libraries(car_gps NemaTode)

add_executable(test_car_gps_reader test_car_gps_reader.cpp)
target_link_libraries(test_car_gps_reader car_gps)

