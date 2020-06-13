#if !defined(CONFIG_LANE_DETECTION_H)
#define CONFIG_LANE_DETECTION_H

#define LANE_DETECTION_BATCH_SIZE 1
#define LANE_DETECTION_N_CLASSES 1
#define LANE_DETECTION_FORCE_REBUILD_ENGINE false
#define LANE_DETECTION_MODEL \
    "models/lane_detection/lane_segmentation_448x448.uff"
#define LANE_DETECTION_TENSORRT_PLAN \
    "models/lane_detection/lane_segmentation_448x448.engine"
#define LANE_DETECTION_USE_FP_16 true
#define LANE_DETECTION_INPUT_WIDTH 448
#define LANE_DETECTION_INPUT_HEIGHT 448
#define LANE_DETECTION_INPUT_NODE "data"
#define LANE_DETECTION_OUTPUT_NODE "sigmoid/Sigmoid"

#endif // CONFIG_LANE_DETECTION_H
