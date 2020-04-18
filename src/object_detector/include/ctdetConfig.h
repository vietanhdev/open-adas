//
// Created by cao on 19-10-26.
//

#ifndef CTDET_TRT_CTDETCONFIG_H
#define CTDET_TRT_CTDETCONFIG_H

namespace ctdet{

    constexpr static float visThresh = 0.5;
    constexpr static int kernelSize = 3;  /// nms maxpool size


    //ctdet  ctdet_coco_dla_2x.onnx
    constexpr static int input_w = 512;
    constexpr static int input_h = 512;
    constexpr static int channel = 3;
    constexpr static int classNum = 10;
    constexpr static float mean[]= {0.408, 0.447, 0.470};
    constexpr static float std[] = {0.289, 0.274, 0.278};
    constexpr static char *className[]= {(char*)"person", (char*)"rider", (char*)"car", (char*)"bus",
                                         (char*)"truck", (char*)"bike", (char*)"motor", (char*)"traffic_light", (char*)"traffic_sign",
                                         (char*)"train"};

}
#endif //CTDET_TRT_CTDETCONFIG_H
