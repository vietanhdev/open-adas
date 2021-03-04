#ifndef TRAFFIC_SIGN_MONITOR_H
#define TRAFFIC_SIGN_MONITOR_H

#include <iostream>
#include <string>
#include "utils/timer.h"
#include "sensors/car_status.h"
#include "perception/object_detection/traffic_sign_classification/sign_classifier.h"
#include "perception/object_detection/object_detector.h"

class TrafficSignMonitor {
   private:
    Timer::time_point_t last_traffic_sign_time;
    std::string last_traffic_sign;
    Timer::time_point_t last_no_traffic_sign_time;
    bool sign_existing = false;

    std::shared_ptr<CarStatus> car_status;

   public:
    
    TrafficSignMonitor(std::shared_ptr<CarStatus> car_status);

    // Get largest traffic sign from traffic objects
    // Return sign type for largest sign or empty string if no sign
    std::string getLargestSign(const std::vector<TrafficObject> &traffic_objects);

    // Update traffic sign
    // Passing sign name if a traffic sign was found
    void updateTrafficSign(const std::vector<TrafficObject> &traffic_objects);

    void triggerSignStatus(std::string sign_type);

};

#endif