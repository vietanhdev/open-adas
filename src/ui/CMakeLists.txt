# Find and link CUDA - A library for model execution on NVIDIA GPU
find_package(CUDA REQUIRED)
if(NOT CMAKE_CUDA_DEVICE_LINK_LIBRARY)
   set(CMAKE_CUDA_DEVICE_LINK_LIBRARY
    "<CMAKE_CUDA_COMPILER> <CMAKE_CUDA_LINK_FLAGS> <LANGUAGE_COMPILE_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
 endif()
if(NOT CMAKE_CUDA_DEVICE_LINK_EXECUTABLE)
   set(CMAKE_CUDA_DEVICE_LINK_EXECUTABLE "<CMAKE_CUDA_COMPILER> <FLAGS> <CMAKE_CUDA_LINK_FLAGS> ${CMAKE_CUDA_COMPILE_OPTIONS_PIC} ${_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS} -shared -dlink <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Setup for Qt GUI
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


# Take care of Qt dependencies
find_package(Qt5 COMPONENTS Core Gui Widgets Test Concurrent Multimedia REQUIRED)

include_directories(
    ".."
    "../sensors/libs/NemaTode/include"
    ${UI_HEADERS}
    ${OpenCV_INCLUDE_DIRS}
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

add_library(
    ui
    "../../resources.qrc"
    "main_window.cpp"
    "main_window.ui"
    "dark_theme/dark_style.qrc"
    "dark_theme/dark_style.cpp"
    "traffic_sign_images.cpp"
    "camera_wizard/camera_wizard.cpp"
    "camera_wizard/instruction_page/instruction_page.cpp"
    "camera_wizard/instruction_page/instruction_page.ui"
    "camera_wizard/measurement_page/measurement_page.cpp"
    "camera_wizard/measurement_page/measurement_page.ui"
    "camera_wizard/four_point_select_page/four_point_select_page.cpp"
    "camera_wizard/four_point_select_page/four_point_select_page.ui"
    "warnings/collision_warning_controller.cpp"
    "warnings/traffic_sign_monitor.cpp"
    
    "simulation/simulation.cpp"
    "simulation/simulation.ui"
    "simulation/can_bus_emitter.cpp"

    "../perception/camera_model/birdview_model.cpp"
    "../perception/camera_model/camera_model.cpp"
)

link_libraries(
    ui
    car_status
    perception
    ${Qt5Widgets_LIBRARIES} 
    ${Qt5Multimedia_LIBRARIES}
)