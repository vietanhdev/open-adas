#ifndef CONFIG_OBJECT_DETECTION_H
#define CONFIG_OBJECT_DETECTION_H

#define SMARTCAM_OBJECT_DETECTION_MODEL \
    "models/object_detection/ctdet_bdd_resnet18_384.onnx"
#define SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN \
    "models/object_detection/ctdet_bdd_resnet18_384.engine"
// Mode: FLOAT32, FLOAT16. INT in the future
#define SMARTCAM_OBJECT_DETECTION_MODE "FLOAT16"
#define MIN_OBJECT_SIZE 10

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
    static std::vector<std::string> className = {"person", "rider", "car", "bus", "truck", "bike", "motor", "traffic_light", "traffic_sign", "train"};
    static std::vector<std::string> drawClassNames = {"person", "rider", "car", "bus", "truck", "bike", "motor"};


}

#endif

