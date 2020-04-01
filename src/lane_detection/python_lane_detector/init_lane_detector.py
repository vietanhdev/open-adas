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

# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
# cap = cv2.VideoCapture('./harder_challenge_video.mp4')

# # Check if camera opened successfully
# if (cap.isOpened() == False):
#     print("Error opening video stream or file")

# # Read until video is completed
# while(cap.isOpened()):
#     # Capture frame-by-frame
#     ret, frame = cap.read()
#     if ret == True:

#         # frame = cv2.rotate(frame, cv2.ROTATE_90_COUNTERCLOCKWISE)
#         # frame = cv2.resize(frame, (ORIGINAL_SIZE))
#         # cv2.imshow("Original", frame)
#         # cv2.waitKey(0)

#         start_time = time.time()
#         result = lane_finder.process_image(frame, False)
#         print("Processing time {}".format(time.time() - start_time))

#         # Display the resulting frame
#         cv2.imshow('Frame', result)

#         # Press Q on keyboard to  exit
#         if cv2.waitKey(25) & 0xFF == ord('q'):
#             break

#     # Break the loop
#     else:
#         break

# # When everything done, release the video capture object
# cap.release()

# # Closes all the frames
# cv2.destroyAllWindows()
