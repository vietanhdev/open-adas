import cv2
import numpy as np
from python_lane_detector import init_lane_detector
import sys

if len(sys.argv) < 2:
    print("Use: python test_lane_detector <path/to/video>")
    exit(1)


video_path = sys.argv[1]
print("Reading video: " + video_path)

lane_detector = init_lane_detector.get_lane_finder()

# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
cap = cv2.VideoCapture(video_path)

# Check if camera opened successfully
if (cap.isOpened()== False): 
  print("Error opening video stream or file")

# Read until video is completed
while(cap.isOpened()):
  # Capture frame-by-frame
  ret, frame = cap.read()
  if ret == True:
    mask = lane_detector.get_lane_mask(frame)

    # Display the resulting frame
    cv2.imshow('Frame',frame)
    cv2.imshow('Result',mask)

    # Press Q on keyboard to  exit
    if cv2.waitKey(25) & 0xFF == ord('q'):
      break

  # Break the loop
  else: 
    break

# When everything done, release the video capture object
cap.release()

# Closes all the frames
cv2.destroyAllWindows()
