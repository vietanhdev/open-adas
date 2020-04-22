import numpy as np
import tensorrt as trt
import pycuda.driver as cuda
import cv2
import os
from utils import *
from math import atan
import colorsys
import time

class LaneDetector:

    INPUT_SIZE = (320, 224)
    OUTPUT_SIZE = (320, 224)

    lane_width = 50
    segment_start = 6
    segment_amount = 1

    finding_lane_again = False
    begin_finding_lane_time = 0

    # Tracking
    last_center = 160
    last_direction = 0
    lane_confidence = 0

    speed_decay = 0


    def __init__(self, onnx_file_path, engine_file_path):
        # Do inference with TensorRT
        trt_outputs = []
        self.engine = get_engine(onnx_file_path, engine_file_path)
        self.context = self.engine.create_execution_context()
        self.inputs, self.outputs, self.bindings, self.stream = allocate_buffers(self.engine)


    def find_main_direction(self, image):
        draw = image.copy()
        draw = cv2.cvtColor(draw, cv2.COLOR_GRAY2BGR)
        linesP = cv2.HoughLinesP(image, 1, np.pi / 180, 50, None, 50, 60)
        sum_x_diff = 0
        if linesP is None:
            return 0, 0 

        for i in range(0, len(linesP)):
            l = linesP[i][0]
            p1 = (l[0], l[1])
            p2 = (l[2], l[3])
            if p1[1] > p2[1]:
                p = p1
                p1 = p2
                p2 = p
            sum_x_diff += p1[0] - p2[0]
            cv2.line(draw, (l[0], l[1]), (l[2], l[3]), (0,0,255), 3, cv2.LINE_AA)

        # print(sum_x_diff)
        # cv2.imshow("lines", draw)

        return np.sign(sum_x_diff), len(linesP)

    def get_lane_mask(self, img):
        input_img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        input_img = cv2.resize(input_img, self.INPUT_SIZE).astype(np.float32)

        # Set host input to the image. The do_inference function will copy the input to the GPU before executing.
        self.inputs[0].host = input_img
        trt_outputs = do_inference(self.context, bindings=self.bindings, inputs=self.inputs, outputs=self.outputs, stream=self.stream)

        # Before doing post-processing, we need to reshape the outputs as the do_inference will give us flat arrays.
        output = trt_outputs[0]
        output = np.reshape((output*255).astype(np.uint8), (self.OUTPUT_SIZE[1], self.OUTPUT_SIZE[0], 3))

        road = output[..., 0].squeeze()
        lane = output[..., 1].squeeze()

        # lane = output[..., 0].squeeze()

        return lane

    def unwarp(self, img, src, dst):
        h, w = img.shape[:2]
        M = cv2.getPerspectiveTransform(src, dst)

        unwarped = cv2.warpPerspective(img, M, (w, h), flags=cv2.INTER_LINEAR)
        return unwarped

    def bird_view(self, source_img, isBridge=False):
        h, w = source_img.shape[:2]
        # define source and destination points for transform

        src = np.float32([(100, 120),
                          (220, 120),
                          (0, 210),
                          (320, 210)])

        dst = np.float32([(120, 0),
                          (w - 120, 0),
                          (120, h),
                          (w - 120, h)])

        src_bridge = np.float32([(50, 180),
                          (270, 180),
                          (0, 210),
                          (320, 210)])

        dst_bridge = np.float32([(80, 0),
                          (w - 80, 0),
                          (80, h),
                          (w - 80, h)])

        if isBridge == True:
            src = src_bridge
            dst = dst_bridge
        # change perspective to bird's view
        unwarped = self.unwarp(source_img, src, dst)
        return unwarped

    