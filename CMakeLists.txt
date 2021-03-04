# Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Specify project title
project(OpenADAS)

# Setup for Qt GUI
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set GPU architecture. This decides which instruction set will be used for GPU code.
set(GPU_ARCHS 75)  ## config your GPU_ARCHS,See [here](https://developer.nvidia.com/cuda-gpus) for finding what maximum compute capability your specific GPU supports.

# Setup CMake
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


# Find and link CUDA - A library for model execution on NVIDIA GPU
find_package(CUDA REQUIRED)
if(NOT CMAKE_CUDA_DEVICE_LINK_LIBRARY)
   set(CMAKE_CUDA_DEVICE_LINK_LIBRARY
    "<CMAKE_CUDA_COMPILER> <CMAKE_CUDA_LINK_FLAGS> <LANGUAGE_COMPILE_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
 endif()
if(NOT CMAKE_CUDA_DEVICE_LINK_EXECUTABLE)
   set(CMAKE_CUDA_DEVICE_LINK_EXECUTABLE "<CMAKE_CUDA_COMPILER> <FLAGS> <CMAKE_CUDA_LINK_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

find_package( OpenCV REQUIRED )

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(
    "src"
    ${CUDA_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDES}
    "/usr/include/x86_64-linux-gnu/qt5/"
    "/usr/include/x86_64-linux-gnu/qt5/QtCore"
    "/usr/include/x86_64-linux-gnu/qt5/QtWidgets" 
    "/usr/include/x86_64-linux-gnu/qt5/QtGui"
    "/usr/include/x86_64-linux-gnu/qt5/QtMultimedia"
    "/usr/include/x86_64-linux-gnu/qt5/QtMultimediaWidgets"
)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# Build the libraries with -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory("src/sensors")
add_subdirectory("src/perception")

# add required source, header, ui and resource files
add_executable(
    ${PROJECT_NAME}
    "src/main.cpp"
    "src/utils/common.cpp"
    "src/utils/file_storage.cpp"

    "resources.qrc"
    "src/ui/main_window.cpp"
    "src/ui/main_window.ui"
    "src/ui/dark_theme/dark_style.qrc"
    "src/ui/dark_theme/dark_style.cpp"
    "src/ui/traffic_sign_images.cpp"
    "src/ui/camera_wizard/camera_wizard.cpp"
    "src/ui/camera_wizard/instruction_page/instruction_page.cpp"
    "src/ui/camera_wizard/instruction_page/instruction_page.ui"
    "src/ui/camera_wizard/measurement_page/measurement_page.cpp"
    "src/ui/camera_wizard/measurement_page/measurement_page.ui"
    "src/ui/camera_wizard/four_point_select_page/four_point_select_page.cpp"
    "src/ui/camera_wizard/four_point_select_page/four_point_select_page.ui"
    "src/ui/warnings/collision_warning_controller.cpp"
    "src/ui/warnings/traffic_sign_monitor.cpp"
    "src/ui/simulation/simulation.ui"
    "src/ui/simulation/simulation.cpp"
    "src/ui/simulation/can_bus_emitter.cpp"

    "src/perception/camera_model/birdview_model.cpp"
    "src/perception/camera_model/camera_model.cpp"
)
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

# Link required libs
target_link_libraries(${PROJECT_NAME} 
    ${CPP_FS_LIB}
    openadas_car_sensors
    openadas_perception
    nvonnxparser
    stdc++fs
    ${Qt5Widgets_LIBRARIES} 
    ${Qt5Multimedia_LIBRARIES}
    ${OpenCV_LIBS}
)

# Copy images, models, sounds and data to dist folder
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

# Setup a virtual can
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/setup_vcan.sh $<TARGET_FILE_DIR:${PROJECT_NAME}>/setup_vcan.sh)
