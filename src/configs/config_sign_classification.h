#if !defined(CONFIG_SIGN_CLASSIFICATION_H)
#define CONFIG_SIGN_CLASSIFICATION_H

#define DEBUG_WRITE_SIGN_CROPS true

#define SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_MODEL \
    "models/traffic_sign/traffic_sign_classification_resnet18_64.uff"
#define SMARTCAM_TRAFFIC_SIGN_CLASSIFICATION_TENSORRT_PLAN \
    "models/traffic_sign/traffic_sign_classification_resnet18_64.engine"
#define SMARTCAM_TRAFFIC_SIGN_CLASS_LIST \
    "models/traffic_sign/classes.txt"

#define MIN_TRAFFIC_SIGN_SIZE 20
#define SIGN_CLASSIFICATION_THRESH 0.8

#endif // CONFIG_SIGN_CLASSIFICATION_H
