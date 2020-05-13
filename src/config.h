#ifndef CONFIG_H
#define CONFIG_H

#define IMG_MAX_SIZE 512 

// #define DISABLE_LANE_DETECTOR
#define SMARTCAM_DEBUG

// Show lane debug images
// #define DEBUG_LANE_DETECTOR_SHOW_LINES
#define DEBUG_LANE_DETECTOR_SHOW_LINE_MASK

#define SMARTCAM_OBJECT_DETECTION_MODEL \
    "models/object_detection/ctdet_bdd_rescdn18.onnx"
#define SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN \
    "models/object_detection/ctdet_bdd_rescdn18.engine"
// Mode: FLOAT32, FLOAT16. INT in the future
#define SMARTCAM_OBJECT_DETECTION_MODE "FLOAT32"

#define SMARTCAM_LANE_DETECTION_MODEL \
    "models/lane_detection/config02_model_.103-0.399921.uff"
#define SMARTCAM_LANE_DETECTION_TENSORRT_PLAN \
    "models/lane_detection/config02_model_.103-0.399921.engine"

#define SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_MODEL \
    "models/traffic_sign/traffic_sign_classification.uff"
#define SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_TENSORRT_PLAN \
    "models/traffic_sign/traffic_sign_classification.engine"
#define SMARTCAM_TRAFFIC_SIGN_CLASS_LIST \
    "models/traffic_sign/classes.txt"


#endif