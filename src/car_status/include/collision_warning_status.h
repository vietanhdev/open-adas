#ifndef COLLISION_WARNING_STATUS_H
#define COLLISION_WARNING_STATUS_H

#include "timer.h"
#include "config.h"

struct CollisionWarningStatus {
    bool is_warning = false;
    bool should_notify = true;
    Timer::time_point_t begin_time;
    Timer::time_point_t notified_time;
};

#endif