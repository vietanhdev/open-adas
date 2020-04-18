"""
This script is used to calibrate the camera based on the provided images
The distortion coefficients and camera matrix are saved for later reuse
"""
import cv2
import os
# import matplotlib.image as mpimg
# import matplotlib.pyplot as plt
import numpy as np
import pickle
from settings import CALIB_FILE_NAME

def calibrate(filename, silent = True):
    images_path = 'camera_cal'
    n_x = 9
    n_y = 6

    # setup object points
    objp = np.zeros((n_y*n_x, 3), np.float32)
    objp[:, :2] = np.mgrid[0:n_x, 0:n_y].T.reshape(-1, 2)
    image_points = []
    object_points = []

    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    # loop through provided images
    for image_file in os.listdir(images_path):
        if image_file.endswith("jpg"):
            # turn images to grayscale and find chessboard corners
            img = cv2.imread(os.path.join(images_path, image_file))
            img_gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
            found, corners = cv2.findChessboardCorners(img_gray, (n_x, n_y))
            if found:
                # make fine adjustments to the corners so higher precision can be obtained before
                # appending them to the list
                cv2.drawChessboardCorners(img, (n_x, n_y), corners, found)
                corners2 = cv2.cornerSubPix(img_gray, corners, (11, 11), (-1, -1), criteria)
                image_points.append(corners2)
                object_points.append(objp)
                if not silent:
                    plt.imshow(img)
                    plt.show()

    # pefrorm the calibration
    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(object_points, image_points, img_gray.shape[::-1], None, None)
    img_size  = img.shape
    # pickle the data and save it
    calib_data = {'cam_matrix':mtx,
                  'dist_coeffs':dist,
                  'img_size':img_size}
    with open(filename, 'wb') as f:
        pickle.dump(calib_data, f, protocol=2)

    if not silent:
        for image_file in os.listdir(images_path):
            if image_file.endswith("jpg"):
                # show distorted images
                #img = mpimg.imread(os.path.join(images_path, image_file))
                plt.imshow(cv2.undistort(img, mtx, dist))
                plt.show()

    return mtx, dist

if __name__ == '__main__':
    calibrate(CALIB_FILE_NAME, True)








