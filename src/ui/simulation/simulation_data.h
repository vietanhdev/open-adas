#ifndef SIMULATION_DATA
#define SIMULATION_DATA

#include <vector>
#include <atomic>
#include <opencv2/opencv.hpp>

struct SimFrameData {
    int begin_frame;
    int end_frame;
    bool turning_left = false;
    bool turning_right = false;
    float car_speed;
    SimFrameData(int begin_frame, int end_frame, float car_speed, bool turning_left, bool turning_right):
    begin_frame(begin_frame), end_frame(end_frame), car_speed(car_speed), turning_left(turning_left), turning_right(turning_right)  {};
};

struct SimulationData {

    float playing_fps = -1;
    int begin_frame = -1;
    int end_frame = -1;

    cv::VideoCapture capture;
    std::vector<SimFrameData> sim_frames;

};


#endif