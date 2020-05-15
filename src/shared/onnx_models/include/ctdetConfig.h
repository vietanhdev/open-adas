//
// Created by cao on 19-10-26.
//

#ifndef CTDET_TRT_CTDETCONFIG_H
#define CTDET_TRT_CTDETCONFIG_H

#include <vector>

namespace ctdet {

    constexpr static float visThresh = 0.4;
    constexpr static int kernelSize = 3;  /// nms maxpool size

    constexpr static int input_w = 384;
    constexpr static int input_h = 384;
    constexpr static int channel = 3;
    constexpr static int classNum = 10;
    constexpr static float mean[]= {0.408, 0.447, 0.470};
    constexpr static float std[] = {0.289, 0.274, 0.278};
    static std::string className[] = {"person", "rider", "car", "bus", "truck", "bike", "motor", "traffic_light", "traffic_sign", "train"};


}
#endif //CTDET_TRT_CTDETCONFIG_H
