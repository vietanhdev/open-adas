#ifndef SPEED_LIMIT_H
#define SPEED_LIMIT_H

#include "timer.h"
#include "config.h"



struct MaxSpeedLimit {
    int speed_limit = -1;

    Timer::time_point_t  begin_time;
    bool has_notified = true; // set this to false to trigger a warning

    bool overspeed_warning = false;
    bool overspeed_warning_has_notified = true;
    Timer::time_point_t  overspeed_warning_notified_time;
};

#endif