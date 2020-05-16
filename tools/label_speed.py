from __future__ import print_function
from __future__ import division
import cv2
import argparse
alpha_slider_max = 240
title_window = 'Speed Labeling'

current_speed = 0

def on_trackbar(val):
    global current_speed
    current_speed = val

parser = argparse.ArgumentParser(description='Label video')
parser.add_argument('--video', help='Path to video')
parser.add_argument('--output_file', help='Path to output file')
args = parser.parse_args()


# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
cap = cv2.VideoCapture(args.video)

# Check if camera opened successfully
if (cap.isOpened()== False): 
  print("Error opening video stream or file")

cv2.namedWindow(title_window, cv2.WINDOW_NORMAL)
cv2.createTrackbar("Speed", title_window , 0, alpha_slider_max, on_trackbar)
# Show some stuff
on_trackbar(0)

speed_data = []
current_frame_id = 0
while(cap.isOpened()):
    # Capture frame-by-frame
    ret, frame = cap.read()
    if ret == True:

        font = cv2.FONT_HERSHEY_SIMPLEX 
        org = (50, 50) 
        fontScale = 1
        color = (255, 0, 0) 
        thickness = 2
        frame = cv2.putText(frame, "{} km/h".format(current_speed), org, font, fontScale, color, thickness, cv2.LINE_AA) 

        # Display the resulting frame
        cv2.imshow(title_window,frame)

        if current_frame_id == 0:
            cv2.waitKey(0)

        speed_data.append("{} {} {}".format(current_frame_id, current_frame_id, current_speed))

        

        # Press Q on keyboard to  exit
        if cv2.waitKey(50) & 0xFF == ord('q'):
            break

    # Break the loop
    else: 
        break

    current_frame_id += 1

# When everything done, release the video capture object
cap.release()

# Closes all the frames
cv2.destroyAllWindows()


with open(args.output_file, "w") as f:
    f.write("\n".join(speed_data))
