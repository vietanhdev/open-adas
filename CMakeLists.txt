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

# To automatically run MOC when building (Meta Object Compiler)
set(CMAKE_AUTOMOC ON)

# To automatically run UIC when building (User Interface Compiler)
set(CMAKE_AUTOUIC ON)

# To automatically run RCC when building (Resource Compiler)
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
find_package(Qt5 COMPONENTS Core Gui Widgets Test Concurrent REQUIRED)

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
    "src/config"
    "src/shared/utils/include"

    "src/user_interface"
    "src/simulation"
    "src/user_interface/dark_theme"

    "src/object_detector"
    "src/shared/onnx_models/include"

    "src/traffic_sign_classifier"
    "src/shared/uff_models/classification_net"

    "src/lane_detector"
    "src/shared/uff_models/common/include"
    "src/shared/uff_models/unet"
    
    "src/car_status/include"

    "src/warning_controller"
    
    ${SDL2_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
)


# Build object detector & tracker
add_subdirectory(src/shared/onnx_models/onnx-tensorrt)
add_subdirectory(src/object_detector)

# Build car prop reader
add_subdirectory(src/car_status)

# Build lane detector
add_subdirectory(src/lane_detector)

# Build sign classifier
add_subdirectory(src/traffic_sign_classifier)

# Build warning controller
add_subdirectory(src/warning_controller)

# add required source, header, ui and resource files
add_executable(${PROJECT_NAME}
    "resources.qrc"
    "src/main.cpp"

    "src/shared/utils/utility.cpp"
    "src/shared/utils/file_storage.cpp"
    "src/shared/utils/timer.cpp"

    "src/user_interface/main_window.cpp"
    "src/user_interface/main_window.ui"
    "src/user_interface/dark_theme/dark_style.qrc"
    "src/user_interface/dark_theme/dark_style.cpp"
    "src/user_interface/traffic_sign_images.cpp"

    "src/simulation/simulation.cpp"
    "src/simulation/simulation.ui"
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
    ${OpenCV_LIBS} 
    ${CPP_FS_LIB} 
    object_detector
    lane_detector
    car_status
    car_gps
    warning_controller
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
