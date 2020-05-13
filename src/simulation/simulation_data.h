#ifndef SIMULATION_DATA
#define SIMULATION_DATA

#include <vector>
#include <atomic>
#include <opencv2/opencv.hpp>

struct SpeedData {
    int begin_frame;
    int end_frame;
    float car_speed;
    SpeedData(int begin_frame, int end_frame, float car_speed):
    begin_frame(begin_frame), end_frame(end_frame), car_speed(car_speed)  {};
};

struct SimulationData {

    float playing_fps = -1;
    int begin_frame = -1;
    int end_frame = -1;

    cv::VideoCapture capture;
    std::vector<SpeedData> speed_data;
    std::vector<float> frame_to_speed;

};


#endif