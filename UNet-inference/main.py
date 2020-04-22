import cv2
import numpy as np
from lane_detector import LaneDetector

lane_detector = LaneDetector(
    "full_model_unet_mobilenet_lane_road.onnx",
    "full_model_unet_mobilenet_lane_road.trt"
)


# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
cap = cv2.VideoCapture('/mnt/DATA/DATASETS/BDD/bdd100k_videos_test_00/bdd100k/videos/test/cabc9045-5a50690f.mov')

# Check if camera opened successfully
if (cap.isOpened()== False): 
  print("Error opening video stream or file")

# Read until video is completed
while(cap.isOpened()):
    # Capture frame-by-frame
    ret, frame = cap.read()
    if ret == True:

        frame = cv2.rotate(frame, cv2.ROTATE_90_COUNTERCLOCKWISE)
        lane_mask = lane_detector.get_lane_mask(frame)

        # Display the resulting frame
        cv2.imshow('lane_mask',lane_mask)
        cv2.imshow('frame',frame)

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

