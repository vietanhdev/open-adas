cmake_minimum_required(VERSION 3.5)
project(car_prop_reader)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -fPIC")

add_subdirectory(libs/NemaTode)     
include_directories(include)                   

file(GLOB CPP_SRC *.cpp)

add_library(car_properties STATIC ${CPP_SRC})
target_link_libraries(car_properties NemaTode)

add_executable(test_car_prop_reader test_car_prop_reader.cpp)
target_link_libraries(test_car_prop_reader car_properties)

