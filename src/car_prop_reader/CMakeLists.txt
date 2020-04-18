cmake_minimum_required(VERSION 3.5)
project(car_prop_reader)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -fPIC")

add_subdirectory(libs/NemaTode)                        

file(GLOB CPP_SRC *.cpp)

add_library(carprop SHARED ${CPP_SRC})
target_link_libraries(carprop NemaTode)

