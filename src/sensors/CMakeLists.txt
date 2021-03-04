set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -fPIC")
SET(CMAKE_C_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE})


add_subdirectory("libs/NemaTode")
include_directories(
    ".."
)

add_library(can_reader 
    can_reader.cpp
    libs/can_lib/can_lib.cpp
)

add_library(openadas_car_sensors car_status.cpp car_gps_reader.cpp)
target_link_libraries(openadas_car_sensors NemaTode can_reader)

add_executable(test_car_gps_reader test_car_gps_reader.cpp)
target_link_libraries(test_car_gps_reader openadas_car_sensors)

