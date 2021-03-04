# Find and link CUDA - A library for model execution on NVIDIA GPU
find_package(CUDA REQUIRED)
if(NOT CMAKE_CUDA_DEVICE_LINK_LIBRARY)
   set(CMAKE_CUDA_DEVICE_LINK_LIBRARY
    "<CMAKE_CUDA_COMPILER> <CMAKE_CUDA_LINK_FLAGS> <LANGUAGE_COMPILE_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
 endif()
if(NOT CMAKE_CUDA_DEVICE_LINK_EXECUTABLE)
   set(CMAKE_CUDA_DEVICE_LINK_EXECUTABLE "<CMAKE_CUDA_COMPILER> <FLAGS> <CMAKE_CUDA_LINK_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

add_subdirectory(common/onnx_models/onnx-tensorrt)
add_subdirectory(object_detection)
add_subdirectory(lane_detection)

include_directories("..")

add_library(
      perception
      "camera_model/birdview_model.cpp"
      "camera_model/camera_model.cpp"
)

target_link_libraries(perception object_detector lane_detector car_sensors)

