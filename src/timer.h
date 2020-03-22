#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <thread>

class Timer {

    public:

    typedef  std::chrono::system_clock::time_point time_point_t;

    typedef long long int time_duration_t;

    time_point_t start_time_point;

    Timer();
    
    static time_point_t getCurrentTime();

    // Calculate the duration between 2 time point
    // return value as time_duration_t (miliseconds)
    static time_duration_t calcDiff(time_point_t begin, time_point_t end);

    // Get time passed (duration from a time point in the past to current time)
    static time_duration_t calcTimePassed(time_point_t time_point);

    // Delay a duration
    static void delay(time_duration_t duration);


};

#endif