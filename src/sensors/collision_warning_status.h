#ifndef COLLISION_WARNING_STATUS_H
#define COLLISION_WARNING_STATUS_H

#include "utils/timer.h"
#include "configs/config.h"

struct CollisionWarningStatus {
    bool is_warning = false;
    bool should_notify = true;
    Timer::time_point_t begin_time;
    Timer::time_point_t notified_time;
};

#endif