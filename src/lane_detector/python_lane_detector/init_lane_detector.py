import cv2
import time
import numpy as np
from lane_detector import LaneDetector

lane_detector = LaneDetector(
    "models/lane_detection/full_model_unet_mobilenet_lane_road.onnx",
    "models/lane_detection/full_model_unet_mobilenet_lane_road.trt"
)

# lane_detector.get_lane_mask(np.zeros((100, 100, 3), dtype=np.uint8))

def get_lane_finder():
    global lane_detector
    return lane_detector