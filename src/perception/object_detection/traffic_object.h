#ifndef TRAFFIC_OBJECT_H
#define TRAFFIC_OBJECT_H

#include <string>
#include "../common/onnx_models/include/ctdet_utils.h"

class TrafficObject {

   public:
    //x1 y1 x2 y2
    Box bbox;
    int classId;
    float prob;
    float distance_to_my_car = -1;
    std::string traffic_sign_type{""}; // Extended type. For traffic sign

    TrafficObject(const Detection &detection, std::string traffic_sign_type) : 
    bbox(detection.bbox), classId(detection.classId), prob(detection.prob), traffic_sign_type(traffic_sign_type) {}
};

#endif // TRAFFIC_OBJECT_H