#include "traffic_sign_monitor.h"

using namespace std;

TrafficSignMonitor::TrafficSignMonitor(std::shared_ptr<CarStatus> car_status) {
    this->car_status = car_status;
}

// Get largest traffic sign from traffic objects
// Return sign type for largest sign or empty string if no sign
std::string TrafficSignMonitor::getLargestSign(const std::vector<TrafficObject> &traffic_objects) {

    int sign_count = 0;
    int max_area = 0;
    std::string largest_sign_type;
    for (size_t i = 0; i < traffic_objects.size(); ++i) {

        std::string sign_type = traffic_objects[i].traffic_sign_type;

        if (sign_type == "" || sign_type == "OTHER") {
            continue;
        }

        // Now we only care about speed sign
        // => Skip others
        if (!TrafficSignClassifier::isSpeedSign(sign_type)) {
            continue;
        }

        int area = (traffic_objects[i].bbox.x2 - traffic_objects[i].bbox.x1) * (traffic_objects[i].bbox.y2 - traffic_objects[i].bbox.y1);

        if (area > max_area) {
            largest_sign_type = sign_type;
            max_area = area;
        }

        sign_count++;

    }

    if (sign_count == 0) {
        return "";
    }

    return largest_sign_type;
}


// Update traffic sign
// Passing sign name if a traffic sign was found
void TrafficSignMonitor::updateTrafficSign(const std::vector<TrafficObject> &traffic_objects) {

    std::string sign_type = getLargestSign(traffic_objects);

    // Traffic sign -> No traffic sign
    if (sign_existing && sign_type == "") {
        last_no_traffic_sign_time = Timer::getCurrentTime();
        sign_existing = false;

    // No traffic sign -> Traffic sign
    } else if (!sign_existing && sign_type != "") {
        last_traffic_sign_time = Timer::getCurrentTime();
        last_traffic_sign = sign_type;
        sign_existing = true;
    
    } else if (sign_existing) {

        if (sign_type != last_traffic_sign) {
            last_traffic_sign_time = Timer::getCurrentTime();
            last_traffic_sign = sign_type;
        } else {
            Timer::time_duration_t traffic_sign_time = Timer::calcTimePassed(last_traffic_sign_time);
            if (traffic_sign_time > 200 && traffic_sign_time < 10000) {    
                // cout << sign_type << endl;     
                triggerSignStatus(sign_type);
            }
        }

    } else {

        Timer::time_duration_t no_traffic_sign_time =  Timer::calcTimePassed(last_no_traffic_sign_time);
        if (no_traffic_sign_time > 1000 && no_traffic_sign_time < 10000) {
            last_traffic_sign = "";
            sign_existing = false;
        }

    }

}


void TrafficSignMonitor::triggerSignStatus(std::string sign_type) {
    
    if (sign_type == "END_OF_SPEED_LIMIT") {
        car_status->removeSpeedLimit();
    } else if (sign_type == "MAX_SPEED_LIMIT_10") {
        car_status->triggerSpeedLimit(10);
    } else if (sign_type == "MAX_SPEED_LIMIT_100") {
        car_status->triggerSpeedLimit(100);
    } else if (sign_type == "MAX_SPEED_LIMIT_110") {
        car_status->triggerSpeedLimit(110);
    } else if (sign_type == "MAX_SPEED_LIMIT_120") {
        car_status->triggerSpeedLimit(120);
    } else if (sign_type == "MAX_SPEED_LIMIT_20") {
        car_status->triggerSpeedLimit(20);
    } else if (sign_type == "MAX_SPEED_LIMIT_30") {
        car_status->triggerSpeedLimit(30);
    } else if (sign_type == "MAX_SPEED_LIMIT_40") {
        car_status->triggerSpeedLimit(40);
    } else if (sign_type == "MAX_SPEED_LIMIT_5") {
        car_status->triggerSpeedLimit(5);
    } else if (sign_type == "MAX_SPEED_LIMIT_50") {
        car_status->triggerSpeedLimit(50);
    } else if (sign_type == "MAX_SPEED_LIMIT_60") {
        car_status->triggerSpeedLimit(60);
    } else if (sign_type == "MAX_SPEED_LIMIT_70") {
        car_status->triggerSpeedLimit(70);
    } else if (sign_type == "MAX_SPEED_LIMIT_80") {
        car_status->triggerSpeedLimit(80);
    } else if (sign_type == "MAX_SPEED_LIMIT_90") {
        car_status->triggerSpeedLimit(90);
    }
    
}
