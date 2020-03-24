#Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Specify project title
project(CarSmartCam)

# Setup for TensorRT
set(GPU_ARCHS 75)  ## config your GPU_ARCHS,See [here](https://developer.nvidia.com/cuda-gpus) for finding what maximum compute capability your specific GPU supports.
# set(TENSORRT_ROOT /usr/local/TensorRT-5.1.5.0)

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

# Specify OpenCV folder, and take care of dependencies and includes
#set(OpenCV_DIR "path_to_opencv")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(SDL2 REQUIRED sdl2)

# Take care of Qt dependencies
find_package(Qt5 COMPONENTS Core Gui Widgets Test Concurrent REQUIRED)

# FIND_PACKAGE(PythonInterp 3.5 REQUIRED)
# FIND_PACKAGE(PythonLibs 3.5 REQUIRED)
# MESSAGE(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
# MESSAGE(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")


# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


# Build object detector
add_subdirectory(src/object_detection/onnx-tensorrt)
add_subdirectory(src/object_detection/ctdet)
add_subdirectory(src/object_detection/inference)

# Build lane detector
# add_subdirectory(src/lane_detection/lane_detector)


include_directories(
    "src"
    "src/utils"
    "src/gui"
    "src/gui/dark"
    "src/object_detection/include"
    "src/lane_detection/include"
    # "src/lane_detection/lane_detector/pyboostcvconverter/include"
    ${PYTHON_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# add required source, header, ui and resource files
add_executable(${PROJECT_NAME}
    "resources.qrc"
    "src/main.cpp"

    "src/utils/utility.cpp"
    "src/utils/file_storage.cpp"
    "src/utils/timer.cpp"

    "src/gui/main_window.cpp"
    "src/gui/main_window.ui"
    "src/gui/dark/dark_style.qrc"
    "src/gui/dark/dark_style.cpp"
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
target_link_libraries(${PROJECT_NAME} ${Qt5Widgets_LIBRARIES} ${OpenCV_LIBS} ${CPP_FS_LIB} ${SDL2_LIBRARIES} ctdet ) # TODO: Add lanedt

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
        ${CMAKE_SOURCE_DIR}/src/lane_detection/lanenet_trt_python $<TARGET_FILE_DIR:${PROJECT_NAME}>/lanenet_trt_python)

