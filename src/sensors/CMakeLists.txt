set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -fPIC")
SET(CMAKE_C_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE})


find_path(TENSORRT_INCLUDE_DIR NvInfer.h
        HINTS ${TENSORRT_ROOT} ${CUDA_TOOLKIT_ROOT_DIR}
        PATH_SUFFIXES include/)

add_subdirectory("libs/NemaTode")
include_directories(
    ".."
    "../perception/common/onnx_models/onnx-tensorrt"
    ${TENSORRT_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIRS}
)

add_library(can_reader 
    can_reader.cpp
    libs/can_lib/can_lib.cpp
)

add_library(openadas_car_sensors car_status.cpp car_gps_reader.cpp)
target_link_libraries(openadas_car_sensors NemaTode can_reader)

add_executable(test_car_gps_reader test_car_gps_reader.cpp)
target_link_libraries(test_car_gps_reader openadas_car_sensors)

