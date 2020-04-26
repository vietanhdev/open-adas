#ifndef CONFIG_H
#define CONFIG_H

#define SMARTCAM_DEBUG

#define SMARTCAM_OBJECT_DETECTION_ONNX_MODEL "models/object_detection/ctdet_bdd_rescdn18.onnx"
#define SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN "models/object_detection/ctdet_bdd_rescdn18.engine"
// Mode: FLOAT32, FLOAT16. INT in the future
#define SMARTCAM_OBJECT_DETECTION_MODE "FLOAT32"

#endif