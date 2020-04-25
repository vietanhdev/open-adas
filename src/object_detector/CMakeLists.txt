cmake_minimum_required (VERSION 3.5)

project(object_detector_with_tracking)

add_subdirectory(detector)
add_subdirectory(tracker)

set(SOURCES
    object_detector_with_tracking.cpp
)

set(HEADERS
    object_detector_with_tracking.h
)

# ----------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------
INCLUDE_DIRECTORIES(
                    ./tracker/src
                    ./tracker/common
                    ./tracker/src/Tracker
                    ./tracker/src/Tracker/HungarianAlg
)

set(LIBS
    ${OpenCV_LIBS}
    ctdet
    mtracking
)

add_library(detector_with_track SHARED ${SOURCES} ${HEADERS})
target_link_libraries(detector_with_track ${LIBS})
