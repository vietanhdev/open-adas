# Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Specify project title
project(CarSmartCam)

# Setup for TensorRT
set(GPU_ARCHS 53)  ## config your GPU_ARCHS,See [here](https://developer.nvidia.com/cuda-gpus) for finding what maximum compute capability your specific GPU supports.

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(CUDA REQUIRED)

if(NOT CMAKE_CUDA_DEVICE_LINK_LIBRARY)
   set(CMAKE_CUDA_DEVICE_LINK_LIBRARY
    "<CMAKE_CUDA_COMPILER> <CMAKE_CUDA_LINK_FLAGS> <LANGUAGE_COMPILE_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
 endif()
if(NOT CMAKE_CUDA_DEVICE_LINK_EXECUTABLE)
   set(CMAKE_CUDA_DEVICE_LINK_EXECUTABLE "<CMAKE_CUDA_COMPILER> <FLAGS> <CMAKE_CUDA_LINK_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

# Specify OpenCV folder, and take care of dependencies and includes
#set(OpenCV_DIR "path_to_opencv")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Take care of Qt dependencies
find_package(Qt5 COMPONENTS Core Gui Widgets Test Concurrent Multimedia REQUIRED)

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


include_directories(
    "src"
    "src/configs"
    "src/common/utils/include"

    "src/user_interface/dark_theme"
    "src/user_interface"
    "src/simulation"
    "src/common/camera_model"
    "src/common/camera_model/camera_wizard"
    "src/common/camera_model/camera_wizard/four_point_select_page"
    "src/common/camera_model/camera_wizard/instruction_page"
    "src/common/camera_model/camera_wizard/measurement_page"

    "src/object_detection"
    "src/common/onnx_models/include"

    "src/object_detection/traffic_sign_classifier"
    "src/common/uff_models/classification_net"

    "src/lane_detection"
    "src/common/uff_models/common/include"
    "src/common/uff_models/unet"
    
    "src/car_sensing/include"
    "src/car_sensing/libs/can_lib"

    "src/warning_controller"
    
    ${CUDA_INCLUDE_DIRS}
)


# Build object detector & tracker
add_subdirectory(src/common/onnx_models/onnx-tensorrt)
add_subdirectory(src/object_detection)

# Build car prop reader
add_subdirectory(src/car_sensing)

# Build lane detector
add_subdirectory(src/lane_detection)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# add required source, header, ui and resource files
add_executable(${PROJECT_NAME}

    "src/main.cpp"
    "resources.qrc"

    "src/common/utils/utility.cpp"
    "src/common/utils/file_storage.cpp"
    "src/common/utils/timer.cpp"

    "src/user_interface/main_window.cpp"
    "src/user_interface/main_window.ui"
    "src/user_interface/dark_theme/dark_style.qrc"
    "src/user_interface/dark_theme/dark_style.cpp"
    "src/user_interface/traffic_sign_images.cpp"

    "src/simulation/simulation.cpp"
    "src/simulation/simulation.ui"
    "src/simulation/can_bus_emitter.cpp"

    "src/common/camera_model/camera_model.cpp"
    "src/common/camera_model/birdview_model.cpp"
    "src/common/camera_model/camera_wizard/camera_wizard.cpp"
    "src/common/camera_model/camera_wizard/instruction_page/instruction_page.cpp"
    "src/common/camera_model/camera_wizard/instruction_page/instruction_page.ui"
    "src/common/camera_model/camera_wizard/measurement_page/measurement_page.cpp"
    "src/common/camera_model/camera_wizard/measurement_page/measurement_page.ui"
    "src/common/camera_model/camera_wizard/four_point_select_page/four_point_select_page.cpp"
    "src/common/camera_model/camera_wizard/four_point_select_page/four_point_select_page.ui"

    "src/warning_controller/collision_warning_controller.cpp"
    "src/warning_controller/traffic_sign_monitor.cpp"

    "src/car_sensing/can_reader.cpp"
)

ADD_LIBRARY(
    ${PROJECTNAME} STATIC
    ${HEADER_FILES}
)

# Use C++ 17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set (CPP_FS_LIB "stdc++fs")
endif()

# Link required libs
target_link_libraries(${PROJECT_NAME} 
    ${Qt5Widgets_LIBRARIES} 
    ${Qt5Multimedia_LIBRARIES} 
    ${OpenCV_LIBS} 
    ${CPP_FS_LIB} 
    object_detector
    lane_detector
    car_status
    car_gps
)

# Copy files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/images $<TARGET_FILE_DIR:${PROJECT_NAME}>/images)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/models $<TARGET_FILE_DIR:${PROJECT_NAME}>/models)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/sounds $<TARGET_FILE_DIR:${PROJECT_NAME}>/sounds)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/setup_vcan.sh $<TARGET_FILE_DIR:${PROJECT_NAME}>/setup_vcan.sh)
