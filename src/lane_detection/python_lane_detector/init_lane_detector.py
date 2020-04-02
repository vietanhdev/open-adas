import cv2
import pickle
import numpy as np
import time
from settings import *
from lane_finder import *

with open(CALIB_FILE_NAME, 'rb') as f:
    calib_data = pickle.load(f)

cam_matrix = calib_data["cam_matrix"]
coefficient_distance_matrix = calib_data["dist_coeffs"]
img_size = calib_data["img_size"]

with open(PERSPECTIVE_FILE_NAME, 'rb') as f:
    perspective_data = pickle.load(f)

perspective_transform = perspective_data["perspective_transform"]
pixel_Per_Metter = perspective_data['pixels_per_meter']
orig_points = perspective_data["orig_points"]


lane_finder = LaneFinder(settings.ORIGINAL_SIZE, settings.UNWARPED_SIZE, cam_matrix,
                         coefficient_distance_matrix, perspective_transform, pixel_Per_Metter)


def get_lane_finder():
    global lane_finder
    return lane_finder