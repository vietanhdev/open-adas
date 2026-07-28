import cv2
import numpy as np

cap = cv2.VideoCapture('build/bin/direct_output.avi')
count = 0
found_green = False
while True:
    ret, frame = cap.read()
    if not ret: break
    # Bounding boxes are drawn with cv::Scalar(0, 255, 0)
    # Check for bright green pixels
    green_mask = (frame[:, :, 0] < 50) & (frame[:, :, 1] > 200) & (frame[:, :, 2] < 50)
    if np.any(green_mask):
        found_green = True
        break
    count += 1
    if count > 1000: break
print("Found bounding boxes:", found_green)
