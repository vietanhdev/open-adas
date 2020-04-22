import colorsys
from math import atan
import numpy as np
import tensorrt as trt
import pycuda.driver as cuda
import pycuda.autoinit
import cv2
import os

TRT_LOGGER = trt.Logger()


def get_engine(onnx_file_path, engine_file_path=""):
    """Attempts to load a serialized engine if available, otherwise builds a new TensorRT engine and saves it."""
    def build_engine():
        """Takes an ONNX file and creates a TensorRT engine to run inference with"""
        with trt.Builder(TRT_LOGGER) as builder, builder.create_network() as network, trt.OnnxParser(network, TRT_LOGGER) as parser:
            builder.max_workspace_size = 1 << 28  # 256MiB
            builder.max_batch_size = 1
            # Parse model file
            if not os.path.exists(onnx_file_path):
                print('ONNX file {} not found, please run yolov3_to_onnx.py first to generate it.'.format(
                    onnx_file_path))
                exit(0)
            print('Loading ONNX file from path {}...'.format(onnx_file_path))
            with open(onnx_file_path, 'rb') as model:
                print('Beginning ONNX file parsing')
                parser.parse(model.read())
            print('Completed parsing of ONNX file')
            print('Building an engine from file {}; this may take a while...'.format(
                onnx_file_path))
            print(">>>>> ", network)
            engine = builder.build_cuda_engine(network)
            print(">>>>> ", engine)
            print("Completed creating Engine")
            with open(engine_file_path, "wb") as f:
                f.write(engine.serialize())
            return engine

    if os.path.exists(engine_file_path):
        # If a serialized engine exists, use it instead of building an engine.
        print("Reading engine from file {}".format(engine_file_path))
        with open(engine_file_path, "rb") as f, trt.Runtime(TRT_LOGGER) as runtime:
            return runtime.deserialize_cuda_engine(f.read())
    else:
        return build_engine()


# Simple helper data class that's a little nicer to use than a 2-tuple.
class HostDeviceMem(object):
    def __init__(self, host_mem, device_mem):
        self.host = host_mem
        self.device = device_mem

    def __str__(self):
        return "Host:\n" + str(self.host) + "\nDevice:\n" + str(self.device)

    def __repr__(self):
        return self.__str__()


def allocate_buffers(engine):
    inputs = []
    outputs = []
    bindings = []
    stream = cuda.Stream()
    # binding_to_type = {"data": np.int32, "sigmoid/Sigmoid": np.float32}
    for binding in engine:
        size = trt.volume(engine.get_binding_shape(
            binding)) * engine.max_batch_size
        dtype = trt.nptype(engine.get_binding_dtype(binding))
        # dtype = binding_to_type[str(binding)]
        # Allocate host and device buffers
        host_mem = cuda.pagelocked_empty(size, dtype)
        device_mem = cuda.mem_alloc(host_mem.nbytes)
        # Append the device buffer to device bindings.
        bindings.append(int(device_mem))
        # Append to the appropriate list.
        if engine.binding_is_input(binding):
            inputs.append(HostDeviceMem(host_mem, device_mem))
        else:
            outputs.append(HostDeviceMem(host_mem, device_mem))
    return inputs, outputs, bindings, stream


# This function is generalized for multiple inputs/outputs.
# inputs and outputs are expected to be lists of HostDeviceMem objects.
def do_inference(context, bindings, inputs, outputs, stream, batch_size=1):
    # Transfer input data to the GPU.
    [cuda.memcpy_htod_async(inp.device, inp.host, stream) for inp in inputs]
    # Run inference.
    context.execute_async(batch_size=batch_size,
                          bindings=bindings, stream_handle=stream.handle)
    # Transfer predictions back from the GPU.
    [cuda.memcpy_dtoh_async(out.host, out.device, stream) for out in outputs]
    # Synchronize the stream
    stream.synchronize()
    # Return only the host outputs.
    return [out.host for out in outputs]


def detect_line_segments(edges):
    # tuning min_threshold, minLineLength, maxLineGap is a trial and error process by hand
    rho = 1  # distance precision in pixel, i.e. 1 pixel
    angle = np.pi / 180  # angular precision in radian, i.e. 1 degree
    min_threshold = 10  # minimal of votes
    line_segments = cv2.HoughLinesP(edges, rho, angle, min_threshold,
                                    np.array([]), minLineLength=8, maxLineGap=4)

    visualize = np.zeros((edges.shape[0], edges.shape[1], 3), dtype=np.uint8)
    visualize[edges > 0] = [255, 255, 255]

    for line_segment in line_segments:
        for x1, y1, x2, y2 in line_segment:
            cv2.line(visualize, (x1, y1), (x2, y2), (0, 0, 255), 2)
    cv2.imshow("visualize", visualize)

    return line_segments


def make_points(frame, line):
    height, width, _ = frame.shape
    slope, intercept = line
    y1 = height  # bottom of the frame
    y2 = int(y1 * 1 / 2)  # make points from middle of the frame down

    # bound the coordinates within the frame
    x1 = max(-width, min(2 * width, int((y1 - intercept) / slope)))
    x2 = max(-width, min(2 * width, int((y2 - intercept) / slope)))
    return [[x1, y1, x2, y2]]


def average_slope_intercept(frame, line_segments):
    """
    This function combines line segments into one or two lane lines
    If all line slopes are < 0: then we only have detected left lane
    If all line slopes are > 0: then we only have detected right lane
    """
    lane_lines = []
    if line_segments is None:
        print('No line_segment segments detected')
        return lane_lines

    height, width = frame.shape[:2]
    left_fit = []
    right_fit = []

    boundary = 1/3
    # left lane line segment should be on left 2/3 of the screen
    left_region_boundary = width * (1 - boundary)
    # right lane line segment should be on left 2/3 of the screen
    right_region_boundary = width * boundary

    for line_segment in line_segments:
        for x1, y1, x2, y2 in line_segment:
            if x1 == x2:
                print('skipping vertical line segment (slope=inf): %s' %
                      line_segment)
                continue
            fit = np.polyfit((x1, x2), (y1, y2), 1)
            slope = fit[0]
            intercept = fit[1]
            if slope < 0:
                if x1 < left_region_boundary and x2 < left_region_boundary:
                    left_fit.append((slope, intercept))
            else:
                if x1 > right_region_boundary and x2 > right_region_boundary:
                    right_fit.append((slope, intercept))

    print("DDDD")
    return

    left_fit_average = np.average(left_fit, axis=0)
    # lane_lines = average_slope_intercept(warped_image, line_segments)
    # return lane_lines

    right_fit_average = np.average(right_fit, axis=0)
    if len(right_fit) > 0:
        lane_lines.append(make_points(frame, right_fit_average))

    # [[[316, 720, 484, 432]], [[1009, 720, 718, 432]]]
    print('lane lines: %s' % lane_lines)

    return lane_lines


def detect_lane(warped_image):

    line_segments = detect_line_segments(warped_image)
    lane_lines = average_slope_intercept(warped_image, line_segments)
    return line_segments


def display_lines(frame, lines, line_color=(0, 255, 0), line_width=2):
    line_image = np.zeros_like(frame)
    if lines is not None:
        for line in lines:
            for x1, y1, x2, y2 in line:
                cv2.line(line_image, (x1, y1), (x2, y2),
                         line_color, line_width)
    line_image = cv2.addWeighted(frame, 0.8, line_image, 1, 1)
    return line_image


def hsv2rgb(h, s, v):
    """
    Convert color in normalized (h, s, v) space into non-normalized (r, g, b) space
    """
    return tuple(round(i * 255) for i in colorsys.hsv_to_rgb(h, s, v))


def bird_view_transform(img, x1=0.43, y1=0.5, x2=0.3, y2=0):
    """
    Transform into bird view image
    """
    h, w = img.shape[:2]
    # define source and destination points for transform

    src = np.float32([
        (x1*w, y1*h),
        ((1-x1)*w, y1*h),
        (0, h),
        (w, h)
    ])

    dst = np.float32([
        (x2*w, y2*h),
        ((1-x2)*w, y2*h),
        (x2*w, (1-y2)*h),
        ((1-x2)*w, (1-y2)*h)
    ])

    # change perspective to bird's view
    M = cv2.getPerspectiveTransform(src, dst)

    img = cv2.warpPerspective(img, M, (w, h), flags=cv2.INTER_LINEAR)
    return img


def draw_debug_img(w, h, angle, list_cnt, list_angles):
    img = np.zeros((h, w, 3)).astype(np.uint8)

    for i, cnt in enumerate(list_cnt):
        angle = list_angles[i]
        h_value = (1 + angle) / 2

        color = hsv2rgb(h_value, 1, 1)
        cv2.drawContours(img, [cnt], -1, color, -1)

    if angle is not None:
        predict_point = (int(w/2 * (1 - angle)), int(h * 0.5))
        middle_point = (int(w/2), int(h * 0.5))
        cv2.line(img, predict_point, middle_point, (0, 192, 0), 5)
        cv2.circle(img, middle_point, 10, (0, 192, 0), -1)
        cv2.circle(img, predict_point, 10, (192, 0, 192), -1)

    return img


def process_image(img, min_cnt_length=100):
    """
    Process bird view image in gray scale
    Return:
        angle: the estimated angle of all the lane, 
                None or float (in range [-1, 1])
        list_cnt: all the contour of lanes in image
        list_angles: the estimated angle of each contour in list_cnt
    """
    EPSILON = 1e-5
    contours, hierarchy = cv2.findContours(
        img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    list_cnt = []
    list_angles = []
    list_weights = []
    for cnt in contours:
        list_cnt_angles = []
        list_cnt_weights = []
        area = cv2.contourArea(cnt)
        list_points = np.transpose(cnt, (0, 2, 1)).squeeze()

        # Filter by length
        if list_points.shape[0] < min_cnt_length:
            continue

        for i in range(1, len(list_points)):
            prev_point = list_points[i-1]
            point = list_points[i]
            angle = atan((point[1] - prev_point[1] + EPSILON) /
                         (point[0] - prev_point[0] + EPSILON))

            list_cnt_angles.append(angle)
            list_cnt_weights.append((prev_point[1] + point[1])/2)

        avg_angle = np.average(list_cnt_angles, weights=list_cnt_weights)
        list_angles.append(avg_angle)
        list_weights.append(area)

        list_cnt.append(cnt)

    angle = None
    if len(list_angles) > 0:
        angle = np.average(list_angles, weights=list_weights)

    return angle, list_cnt, list_angles


font = cv2.FONT_HERSHEY_SIMPLEX


def Pre(frame_):
    frame = frame_[100:320, :]
    black = np.zeros((220, 480), np.uint8)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    mean = np.mean(gray)
    adj = mean*0.7
    if adj > 80:
        adj = 80
    if adj < 20:
        adj = 20
    mask_white = cv2.inRange(gray, mean+adj, 255)
    #cv2.imshow("mask_white", mask_white)
    adap = cv2.adaptiveThreshold(
        gray, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY, 201, -50)
    #cv2.imshow("adap", adap)
    bitor = cv2.bitwise_or(mask_white, adap)
    kernel = np.ones((5, 5), np.uint8)
    closing = cv2.morphologyEx(bitor, cv2.MORPH_CLOSE, kernel)
    _, contours, __ = cv2.findContours(
        closing, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    for cnt in contours:
        x, y, w, h = cv2.boundingRect(cnt)
        if w*2+h*2 > 280:
            cv2.drawContours(black, [cnt], -1, 255, -1)
    # cv2.imshow("black",black)
    return black


def find_lane_classic(frame_, side_tracking, mode, dis, center_old, x):
    x1 = 320-x
    x2 = 320
    frame = frame_.copy()
    if side_tracking == 1:
        center_old = 480 - center_old
        frame = cv2.flip(frame, 1)
    img = frame.copy()  # img show
    frame = cv2.resize(frame, (480, 320))
    frame = Pre(frame)
    img2 = frame.copy()
    frame = frame[frame.shape[0]-x:frame.shape[0], :]
    if mode == 0:
        _, contours, __ = cv2.findContours(
            frame, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        obj = np.zeros((2, 4))  # obj[row,col]
        stt = 0
        center = center_old
        scnt = 0
        for cnt in contours:
            xct, yct, wct, hct = cv2.boundingRect(cnt)
            #cv2.putText(img,str(wct)+" "+str(hct), (xct, yct+250), font, 1,(255,255,255),3,cv2.LINE_AA)
            rect = cv2.minAreaRect(cnt)
            box = cv2.boxPoints(rect)
            box = np.int0(box)
            box2 = box.copy()
            box2[:, 1] += 320 - x
            block_w = np.sqrt((box2[0][0]-box2[1][0]) **
                              2+(box2[0][1]-box2[1][1])**2)
            block_h = np.sqrt((box2[0][0]-box2[3][0]) **
                              2+(box2[0][1]-box2[3][1])**2)
            if block_w > block_h:
                stemp = block_h
                block_h = block_w
                block_w = stemp
            scnt = cv2.contourArea(cnt)
            if (scnt < 5000) and (block_w > 10) and (block_w < 80) and (block_h > 50) and (stt < 3):
                cv2.drawContours(img, [box2], 0, (255, 130, 171), -1)
                dis_01 = np.sqrt((box2[0][0]-box2[1][0])
                                 ** 2+(box2[0][1]-box2[1][1])**2)
                dis_03 = np.sqrt((box2[0][0]-box2[3][0])
                                 ** 2+(box2[0][1]-box2[3][1])**2)
                # if box[1][1]<box[3][1]:
                if dis_01 > dis_03:
                    center_high = int((box[1][0]+box[2][0])/2)
                    center_low = int((box[0][0]+box[3][0])/2)
                else:
                    center_high = int((box[3][0]+box[2][0])/2)
                    center_low = int((box[0][0]+box[1][0])/2)
                obj[:, stt] = center_high, center_low
                cv2.putText(img, str(int(block_w))+"x"+str(int(block_h)),
                            (center_high-30, 220), font, 1, (255, 255, 255), 3, cv2.LINE_AA)
                #cv2.circle(img,(center_high,x1), 5, (96, 164, 244), -1)
                #cv2.circle(img,(center_low,x2), 5, (96, 164, 244), -1)
                stt += 1

        if stt == 1:
            if obj[1][0] <= 380:
                center = int(obj[0, 0]) + dis
        if stt == 2:
            lane1 = int(obj[0, 0])
            lane2 = int(obj[0, 1])
            if (obj[1][0]-380)*(obj[1][1]-380) < 0:  # 2 line khac phia
                center = min(lane1, lane2) + dis
            else:
                center = max(lane1, lane2) + dis
    else:
        frame = frame[0:20, :]
        # cv2.imshow("frame",frame)
        _, contours, __ = cv2.findContours(
            frame, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        obj = np.zeros((4, 4))  # obj[row,col]
        stt = 0
        center = center_old
        x_min = 1000
        x_max = 0
        xlim = 420
        save = np.zeros((4, 1))
        if mode == 1:
            for cnt in contours:
                _x, _y, _w, _h = cv2.boundingRect(cnt)
                # if w>10:
                if _x < x_min and _x < xlim:
                    x_min = _x
                    save = _x, _y, _w, _h
            center = x_min + dis
            cv2.rectangle(img, (save[0], save[1]+x1-10), (save[0] +
                                                          save[2], save[1]+save[3]+x1-10), (124, 243, 196), -1)
        else:
            for cnt in contours:
                _x, _y, _w, _h = cv2.boundingRect(cnt)
                # if w>10:
                if _x+_w > x_max and _x+_w < xlim:
                    x_max = _x+_w
                    save = _x, _y, _w, _h
            center = x_max + dis
            cv2.rectangle(img, (save[0], save[1]+x1-10), (save[0] +
                                                          save[2], save[1]+save[3]+x1-10), (124, 243, 196), -1)
        cv2.line(img, (xlim, 250), (xlim, 360), (255, 0, 0), 5)
    # cv2.rectangle(img,(0,x1),(480,x2),(255,0,0),1)
    #cv2.putText(img,str(lane_old),(240, 100), font, 1,(255,255,255),3,cv2.LINE_AA)
    cv2.circle(img, (center, 250), 10, (0, 255, 0), -1)
    if side_tracking == 1:
        center = 480 - center
        img = cv2.flip(img, 1)
    return img, img2, center
