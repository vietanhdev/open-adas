#ifndef CONFIG_H
#define CONFIG_H

// #define DISABLE_LANE_DETECTOR
#define SMARTCAM_DEBUG

#define SMARTCAM_OBJECT_DETECTION_MODEL "models/object_detection/ctdet_bdd_rescdn18.onnx"
#define SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN "models/object_detection/ctdet_bdd_rescdn18.engine"
// Mode: FLOAT32, FLOAT16. INT in the future
#define SMARTCAM_OBJECT_DETECTION_MODE "FLOAT32"

#define SMARTCAM_LANE_DETECTION_MODEL "models/lane_detection/full_model_mapillary_lane_resnet18.uff"
#define SMARTCAM_LANE_DETECTION_TENSORRT_PLAN "models/lane_detection/full_model_mapillary_lane_resnet18.engine"

#endif