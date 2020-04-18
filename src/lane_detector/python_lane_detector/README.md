**Advanced Lane Finding Project**

The goals / steps of this project were the following:

* Compute the camera calibration matrix and distortion coefficients given a set of chessboard images.
* Apply a distortion correction to raw images.
* Use color transforms, gradients, etc., to create a thresholded binary image.
* Apply a perspective transform to rectify binary image ("birds-eye view").
* Detect lane pixels and fit to find the lane boundary.
* Determine the curvature of the lane and vehicle position with respect to center.
* Warp the detected lane boundaries back onto the original image.
* Output visual display of the lane boundaries and numerical estimation of lane curvature and vehicle position.

[//]: # (Image References)

[image1]: ./readme_images/calibration.jpg "Original image"
[image2]: ./readme_images/corners.jpg "Image with corners"
[image3]: ./readme_images/undistorted.jpg "Undistorted image"
[image4]: ./readme_images/pinhole.png "Undistorted image"
[image5]: ./readme_images/projective.png "Example of linear perspective"
[image6]: ./readme_images/undistorted_vp.jpg "Undistorted image"
[image7]: ./readme_images/edges_vp.jpg "Edge image"
[image8]: ./readme_images/lines_vp.jpg "Image with lines"
[image9]: ./readme_images/lines.png "Illustration of line vanshing point calculation"
[image10]:./readme_images/trapezoid.jpg "Image with trapezoid and vanishing point"
[image11]:./readme_images/warped.jpg "Warped original image"
[image12]:./readme_images/mask.jpg "Thresholded luminesence channel"
[image13]:./readme_images/with_lines.jpg "Image with lanes"
[image14]: ./readme_images/undistorted_cf.png "Warp Example"
[image15]: ./readme_images/warped_cf.png "Fit Visual"
[image16]: ./test_images/test5.jpg "Initial image"
[image17]: ./readme_images/undistorted_si.jpg "Undistorted single image"
[image18]: ./readme_images/warped_si.jpg "Warped"
[image19]: ./readme_images/llab.jpg "Undistorted single image"
[image20]: ./readme_images/lhls.jpg "Warped"
[image21]: ./readme_images/blab.jpg "Initial image"
[image22]: ./readme_images/llab_thresh.jpg "Undistorted single image"
[image23]: ./readme_images/lhls_thresh.jpg "Warped"
[image24]: ./readme_images/blab_thresh.jpg "Initial image"
[image25]: ./readme_images/total_mask.jpg "Undistorted single image"
[image26]: ./readme_images/eroded_mask.jpg "Undistorted single image"
[image27]: ./readme_images/line_masks.jpg "Undistorted single image"
[image28]: ./readme_images/lines.jpg "Undistorted single image"
[image29]: ./readme_images/warped_lanes.jpg "Undistorted single image"
[image30]: ./output_images/test5.jpg "Undistorted single image"

##Camera Calibration
Before starting the implementation of the lane detection pipeline, the first thing that should be done is camera calibration. That would help us:
* undistort the images coming from camera, thus improving the quality of geometrical measurement
* estimate the spatial resolution of pixels per meter in both *x* and *y* directions

For this project, the calibration pipeline is implemented in function `calibrate` implemented in file `calibrate.py`. The procedure follows these steps:

The procedures start by preparing "object points", which will be the (x, y, z) coordinates of the chessboard corners in the world. Here I am assuming the chessboard is fixed on the (x, y) plane at z=0. Also,  I am assuming that the size of the chessboard pattern is the same for all images, so the object points will be the same for each calibration image.

Then, the calibration images are sequentially loaded, converted to grayscale and chessboard pattern is looked for using `cv2.findChessboardCorners`. When the pattern is found, the positions of the corners get refined further to sub-pixel accuracy using `cv2.cornerSubPix`. That improves the accuracy of the calibration. Corner coordinates get appended to the list containing all the image points, while prepared "object points" gets appended to the list containing all the object points. 

The distortion coefficients and camera matrix are computed using `cv2.calibrateCamera()` function, where image and object points are passed as inputs. It is important to check if the result is satisfactory, since calibration is a nonlinear numerical procedure, so it might yield suboptimal results. To do so calibration images are read once again and undistortion is applied to them. The undistorted images are shown in order to visually check if the distortion has been corrected. Once the data has been checked the parameters are is pickled saved to file. One sample of the input image, image with chessboard corners and the undistorted image is shown:

| Original image     | Chessboard corners | Undistorted image  |
|--------------------|--------------------|--------------------|
|![alt text][image1] |![alt text][image2] |![alt text][image3] |

Before we move further on, lets just reflect on what the camera matrix is. The camera matrix encompases the pinhole camera model in it. It gives the relationship between the coordinates of the points relative to the camera in 3D space and position of that point on the image in pixels. If *X*, *Y* and *Z* are coordinates of the point in 3D space, its position on image (*u* and *v*) in pixels is calculated using:

<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\begin{bmatrix}u\\v\\1\end{bmatrix}=s\mathbf{M}\begin{bmatrix}X\\Y\\Z\end{bmatrix}" alt="{https://latex.codecogs.com/svg.latex?\begin{bmatrix}u\\v\\1\end{bmatrix}=s\mathbf{M}\begin{bmatrix}X\\Y\\Z\end{bmatrix}}">
</p>

where **M** is camera matrix and *s* is scalar different from zero. This equation will be used later on.

| Pinhole camera model          |
|-------------------------------|
|![Pinhole camera model][image4]|


##Finding projective transformation
Next step is to find the projective transform so the original images can be warped so that it looks like the camera is placed directly above the road. OOne approach is to hand tune the source and destination points, which are required to compute the transformation matrix. On the other hand, the script that does that for us can be created based on linear perspective geometry. Let's look at the perspective geometry on the renaissance painting "Architectural Veduta" by Italian painter Francesco di Giorgio Martini. It is easy to note that all lines meet at a single point called vanishing point. The second thing to note is that the square floor tiles centered horizontally in the image, appear as trapezoids with horizontal top and bottom edges and side edges radiating from the vanishing point.  

| Architectural Veduta          |
|-------------------------------|
|![Architectural Veduta][image5]|

Our goal is to achieve exactly opposite, to transform a trapezoidal patch of the road in front of the car to a rectangular image of the road. To do so, the trapezoid needs to be defined as previously noted, horizontal top and bottom centered with respect to a vanishing point, sides radiating from the vanishing point. Of course, to define that, the first task is to find the vanishing point. 

The vanishing point is the place where all parallel lines meet, so to find it we will be using images with straight lines `straight_lines1.jpg`, `straight_lines2.jpg`. First, the images are undistorted, the Canny filter is applied and most prominent lines are identified using `cv2.HoughLinesP`. These images show how the pipeline works:

| Undistorted image  | Edges              | Image with lines   |
|--------------------|--------------------|--------------------|
|![alt text][image6] |![alt text][image7] |![alt text][image8] |

All detected lines are added to a list. The vanishing point is at the intersection of all the lines from the list. Unfortunately, when more than two lines are present, the unique intersecting point might not exist. To overcome that the vanishing point is selected as the point whose total squared distance from all the lines is minimal, thus optimization procedure will be employed. Each line found by the Hough lines can be represented by the point on it **p**<sub>i</sub> and  unit normal to it **n**<sub>i</sub>. Coordinate of the vanishing point is **vp**. So the total squared distance (and a cost function to be minimized is):
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?I=\frac{1}{2}\sum\left(\mathbf{n}_i^T(\mathbf{vp}-\mathbf{p}_i)\right)^2" alt="{mathcode}">
</p>

To find the minimum the cost function is differentiated with respect to the **vp**. After some derivation the following is obtained:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\frac{\partial{I}}{\partial\mathbf{vp}}=0\implies\left(\sum\mathbf{n}_i\mathbf{n}_i^T\right)\mathbf{vp}=\left(\sum\mathbf{n}_i\mathbf{n}_i^T\mathbf{p}_i\right)\implies\mathbf{vp}=\left(\sum\mathbf{n}_i\mathbf{n}_i^T\right)^{-1}\left(\sum\mathbf{n}_i\mathbf{n}_i^T\mathbf{p}_i\right)" alt="{mathcode}">
</p>

Once the vanishing point is found, the top and bottom are defined manually and the trapezoid edges can be calculated. The corners of the trapezoid are used as a source points, while destination points are four corners of the new image. The size of the warped image is defined in file `settings.py`. After that, the matrix that defines the perspective transform is calculated using `cv2.getPerspectiveTransform()`. The procedure that implements the calculation of homography matrix of the perspective transform is implemented at the beginning of the python script `find_perspective_transform.py` (lines 9 - 67). Images that illustrate the procedure follow. Please note that bottom points of the trapezoid are outside of the image, what is the reason for black triangles shown in the warped image.

| Finding VP with multiple lines |Trapezoid and vanishing point|     Warped image   |
|--------------------------------|-----------------------------|--------------------|
|![alt text][image9]             |![alt text][image10]         |![alt text][image11]|

The obtained source and destination points are:

| Source        | Destination   |
|:-------------:|:-------------:|
| 375, 480      | 0, 0          |
| 905, 480      | 500, 0        |
| 1811, 685     | 500, 600      |
| -531, 685     | 0, 600        |

The selected range is quite big, but that had to be done in order to be able to find the lanes on the harder challenge video. In that video, the bends are much sharper than on the highway and might easily veer outside of the trapezoid causing the whole pipeline to fail.

Once again, lets just reflect on what is the matrix returned by the `cv2.getPerspectiveTransform()`. It tells how the perspective transformation is going to be performed and where the pixel from original image  with the coordinates (*u*, *v*) is going to move after the transformation. The destination of that pixel on the warped image would be the point with the coordinates (*u<sub>w</sub>*, *v<sub>w</sub>*). The new position is calculated using::

<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\begin{bmatrix}u_w\\v_w\\1\end{bmatrix}=s\mathbf{H}\begin{bmatrix}u\\v\\1\end{bmatrix}" alt="{mathcode}">
</p>

where **H** is homography matrix returned as a result of `cv2.getPerspectiveTransform()` and *s* is scalar different from zero. 


##Estimating pixel resolution
Next important step is to estimate resolution in pixels per meter of the warped image. It also can be done by hand, but as previously we'll create a script that does that for us. In the course materials, it was stated that width of the lane is no less than 12 feet. In order to estimate the resolution in pixels per meter, the images with the straight lines will be used. They will be unwarped and the distance between the lines will be measured. The lower of two distances will be assumed to be 12 feet or 3.6576 meters. 

To start, the images with the straight lines would be unwarped and color would be converted to HLS space. To find the lanes the threshold would be applied to the luminesence component. Also, only some region of interest is taken into account. Since lines were straight heading towards the vanishing point, after the warping they will be vertical. The centroids of the blobs on left and right images would be calculated using image moments and function `cv2.moments()`. Since the lane lines are vertical the width of the lane in pixels is the difference between the *x* coordinates of two centroids. That allows for the calculation of the width in pixels and then resolution in *x* direction. This procedure is implemented between line 71 and 
91 of the script `find_perspective_transform.py`. The images that illustrate the procedure are shown below.

| Warped image with parallel lane lines| Thresholded luminesence  |Lane with lines identified|
|--------------------------------------|--------------------------|--------------------------|
|![alt text][image11]                  |![alt text][image12]      |![alt text][image13]      |

That is how to find resolution in *x* direction, but for finding resolution in *y* there is no such trick. Since nothing was estimated manually neither this will be. The camera matrix obtained by calibrations holds some relative information about resolutions in *x* and *y* direction. We can try to exploit that. To find resolution in *y* direction we have to do some more math. 

Lets say, we have a coordinate frame attached to the road, as shown on image below. It is easy to see that transformation of the coordinates represented in the coordinate frame of the road to the coordinate frame of the warped image, consists of scaling and shifted. Scale in *x* direction corresponds to the number of pixel per meter in *x* direction. Same holds for *y*. In mathemathical form that can be written as:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\begin{bmatrix}u_w\\v_w\\1\end{bmatrix}=\begin{bmatrix}r_x&0&c_x\\0&r_y&c_y\\0&0&1\end{bmatrix}\begin{bmatrix}X_r\\Y_r\\1\end{bmatrix}" alt="{mathcode}">
</p>

| Road frame as seen by camera| Road frame on warped image |
|-----------------------------|----------------------------|
|![alt text][image14]         |![alt text][image15]        |


The same thing can be calculated from the other side. Lets say that position and of the road coordinate frame in camera coordinate frame is given with rotation matrix **R**=*[r<sub>1</sub> r<sub>2</sub> r<sub>3</sub>]* and translation vector *t*. One important property that is going to be exploited is that matrix **R** is orthogonal, meaning that each of the rows *r<sub>1</sub>, r<sub>2</sub>, r<sub>3</sub>* has the length of 1. Now since we know that, the pixel on the image that corresponds to the point with coordinates *X<sub>r</sub>, Y<sub>r</sub>* and *Z<sub>r</sub>* is calculated by:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\begin{bmatrix}u\\v\\1\end{bmatrix}=s\mathbf{M}\left[r_1\;r_2\;r_3\;t\right]\begin{bmatrix}X_r\\Y_r\\Z_r\\1\end{bmatrix}" alt="{mathcode}">
</p>

Since the road is planar, the *Z<sub>r</sub>=0*. Now we apply the perspective transform and get:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?\begin{bmatrix}u_w\\v_w\\1\end{bmatrix}=s\mathbf{H}\mathbf{M}\left[r_1\;r_2\;t\right]\begin{bmatrix}X_r\\Y_r\\1\end{bmatrix}=\begin{bmatrix}r_x&0&c_x\\0&r_y&c_y\\0&0&1\end{bmatrix}\begin{bmatrix}X_r\\Y_r\\1\end{bmatrix}" alt="{mathcode}">
</p>
Since it has to hold for every point we can conclude that:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?s\left[r_1\;r_2\;t\right]=(\mathbf{H}\mathbf{M})^{-1}\left\begin{bmatrix}r_x&0&c_x\\0&r_y&c_y\\0&0&1\end{bmatrix}=[h_1\;h_2\;h_3]\begin{bmatrix}r_x&0&c_x\\0&r_y&c_y\\0&0&1\end{bmatrix}" alt="{mathcode}">
</p>

Where *h<sub>1</sub>, h<sub>2</sub>, h<sub>3</sub>* are columns of matrix (**HM**)*<sup>-1*. Since the length of vectors *r<sub>1</sub>* and *r<sub>2</sub>* is one, we can calculate scalar *s* and finally *r<sub>y</sub>*:
<p align="center">
<img src="https://latex.codecogs.com/svg.latex?r_y=r_x\frac{\left||h_1\right||}{\left||h_2\right||}" alt="{mathcode}">
</p>

The rather simple equation is obtained at the end, but it took us a while to get there. This calculation is implemented in lines 91 and 92 of script `find_perspective_transform.py`. The final result is:


| pix/meter in *x* direction | pix/meter in *y* direction | 
|----------------------------|----------------------------|
|46.567                         |      33.0652548749         |

----

## Finding lane lines

The pipeline is implemented in the class `LaneFinder` that does the lane finding and it is written in the file `lane_finder.py`. This class has two instances of subclass `LaneLineFinder` which is used to find a single line. The parts of the pipeline that have to be performed once (masking, calculating curvature, drawing an overlay etc...) on a whole image are encapsulated in the `LaneFinder`. The parts that have to be performed twice (fitting the line on a binary image), once for each line is encapsulated in `LaneLineFinder`. For easier explanation first, the functionality used for the single images will be explained. The pipeline for the video is almost the same, while some additional filtering is included.



### Single images
The pipeline for single images goes through several steps. Let us see how initial image looks:
![alt text][image16]

####1. Image undistortion, warping and color space conversion.
The image gets undistorted first, then the perspective transformation is applied to the undistorted image. After that, the images is converted into HLS and LAB color space. L channels of both HLS and LAB channels will be used to track the bright regions of the image, while B channel is used to track the yellow lines on the image. Also, some filtering is performed to reduce the noise, `cv2.medianBlur()` is used since it maintains the edges. Also for the use of the harder challenge video, the greenish areas are unmasked. Part of the code that performs this is from line 187 to 220. The undistorted and warped images are:

| Undistorted original image  | Warped undistorted image   |
|-----------------------------|----------------------------|
|![alt text][image17]         |![alt text][image18]        |

####2. Finding bright or yellow areas
To find the bright or yellow areas, the morphological TOP HAT operation is used. It isolates the areas brighter than their surroundings. This operation is used in order to make pipeline robust against the lighting changes. In selected case, the lightness of the road surface changes, but we'll see that it does not affect the tophat morphological operation. The edge detection not used since they are extremely affected by the noise on the image, which makes them unsuitable for harder challenges. After applying TOP HAT operation, the image is thresholded using adaptive threshold which adds a bit more to the overall robustness. This part is implemented in lines 225 to 237. The resulting images are:

| Tophat on L channel from LAB| Tophat on L channel from HLS  |Tophat on B channel from LAB|
|-----------------------------|-------------------------------|----------------------------|
|![alt text][image19]         |![alt text][image20]           |![alt text][image21]        |
| Tophat on L channel from LAB| Tophat on L channel from HLS  |Tophat on B channel from LAB|
|![alt text][image22]         |![alt text][image23]           |![alt text][image24]        |


####3. Calculating single mask and passing it to the `LaneLineFinder`
Once the masks are calculated, logical *or* is applied between them in order to obtain the total mask. Since the threshold is kept quite low, there will be a lot of noise. To avoid noise interfering with lane finding procedure, the mask is eroded which removes all the regions smaller than the structuring element. Once that is finished the masks are passed to `LaneLineFinder` which actually looks for the line in the binary image. This part is implemented from line 236 to line 248. The results are:

| Total mask           |     Eroded mask      |
|----------------------|----------------------|
|![alt text][image25]  |![alt text][image26]  |

####4. Finding the line in a mask and fitting polynomial
When the mask is found, the search for the line begins. The initial point to start a search is somewhere 1.82 meters (6 feet). Under the assumption that lane is 12feet wide and that the car is in its middle, we would be spot on. Since that might not hold, the search is performed in its surroundings. The window at the bottom of the image with the highest number of points included found. After that, we go one layer up and perform the same search but right now, we start from the maximum from the layer below. The search is performed until the top of the image is reached, gradually eliminating points outside of the maximal region. Function that does this is `LaneLineFinder.find_lane_line()`. After the lane points have been isolated, the polynomial fit is performed using `LaneLineFinder.fit_lane_line()`. In that procedures some statistics are calculated which help determine if the found line is good or not. The statistics include:
 1. Number of points in the fit 
 2. Quality of the fit calculated using covariance matrix returned by `numpy.polyfit`
 3. Curvature of the road
 4. How far lane is from the center of the car

All of these have to be above some empirically defined threshold. The maximal regions and points used for fitting are shown in the image below (red is for left, blue is for right line):

| Line mask            |     Line points      |
|----------------------|----------------------|
|![alt text][image27]  |![alt text][image28]  |

####5. Draw lane on original image and calculate curvature
If lanes are found, the curvature and position of the centre of the car is calculated using functions `get_curvature()` and `get_center_shift()`. Since the two lines are present, for the coefficient of the polynomial the mean value is used. The *y* coordinate for which the polynomials are evaluated is the bottom of the image. After that, the lines are drawn on a warped image and then unwarped and added to the original image. The last thing is to print out the values of curvature and offset of the center of the car. Results for all provided images can be found in [output_images](./output_images). Here is the result for discussed case:

| Warped lines         |     Final result    |
|----------------------|----------------------|
|![alt text][image29]  |![alt text][image30]  |

###Videos 
For the videos, the pipeline follows the basic pipeline applied to single images. Additionally, because of the temporal dimension, some additional filtering is applied. Here is what is done:
 1. The polynomial coefficients are averaged over last 7 iterations. That helps make the lane lines smoother and procedures more robust (line 88). 
 2. The lane has to be in close proximity to its previous position. That helps us narrow the search and avoid looking for windows with a maximum number of points in it (lines 132 -138).
 3. The right lane has to be approximately 12ft (3.6m) right from the left lane (lines 146-147, 243- 246)
 4. Left and right lane should have similar curvature and angle (lines 180-186, 251-252). 

The pipeline is run on all three provided videos. It works great for `project_video.mp4` and `challenge_video.mp4`. It works with `harder_challenge_video.mp4` as well but loses the lane a few times. 
Here are links to the videos:

 1. [project_video](./output_videos/lane_project_video.mp4)
 2. [challenge_video](./output_videos/lane_challenge_video.mp4)
 3. [harder_challenge_video](./output_videos/lane_harder_challenge_video.mp4)

---

##Discussion

The biggest issue by far for me were sudden changes of light conditions. In those cases, the lines get either completely lost (going from bright to dark) or image gets filled with noise coming from the white spots. Although I have done the best I can to make pipeline robust against that kind of changes, they still can cause major problems, which is evident from [harder challenge video](./output_videos/lane_harder_challenge_video.mp4). More advanced filtering and brightness equalization techniques have to be examined. 
 
The averaging out of polynomial coefficients over the last couple of iterations is inappropriate. Effects of it can be in [harder_challenge_video](./output_videos/lane_harder_challenge_video.mp4) where lane computed by the code lags behind the lane on the image, especially in the case of sharp bends. Some better filtering technique has to be applied.

