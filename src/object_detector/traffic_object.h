#ifndef TRAFFIC_OBJECT_H
#define TRAFFIC_OBJECT_H

#include <string>
#include "ctdet_utils.h"

class TrafficObject {

   public:
    //x1 y1 x2 y2
    Box bbox;
    int classId;
    float prob;
    std::string extended_type{""}; // Extended type. For traffic sign

    TrafficObject(const Detection &detection, std::string extended_type) : 
    bbox(detection.bbox), classId(detection.classId), prob(detection.prob), extended_type(extended_type) {}
};

#endif