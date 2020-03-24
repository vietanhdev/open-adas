cmake_minimum_required(VERSION 3.5)
project(lanedt_trt)

find_package(OpenCV REQUIRED)
link_directories(${OpenCV_LIBRARIES_DIRS})

find_package(Boost REQUIRED)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

file(GLOB CPP_SRC *.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast")

include_directories(${OpenCV_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIR}
                    pyboostcvconverter/include
                    ../include)

add_subdirectory(pyboostcvconverter)

add_library(lanedt SHARED ${CPP_SRC})
target_link_libraries(lanedt ${OpenCV_LIBS} ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
