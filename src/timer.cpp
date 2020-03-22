#include "timer.h"
#include <iostream> 

Timer::Timer() {
    start_time_point = getCurrentTime();
}

Timer::time_point_t Timer::getCurrentTime() {
    return std::chrono::system_clock::now();
}

// Calculate the duration between 2 time point
// return value as time_duration_t (miliseconds)
Timer::time_duration_t Timer::calcDiff(time_point_t begin, time_point_t end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

// Get time passed (duration from a time point in the past to current time)
Timer::time_duration_t Timer::calcTimePassed(time_point_t time_point) {
    return calcDiff(time_point, getCurrentTime());
}

// Delay a duration
void Timer::delay(time_duration_t duration) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}