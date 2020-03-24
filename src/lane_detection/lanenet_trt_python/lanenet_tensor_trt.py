#!/usr/bin/env python3

import argparse
import numpy as np
import tensorflow as tf
import time
import cv2
from PIL import Image


import tensorflow.contrib.tensorrt as trt

MAX_BATCH_SIZE = 1
MAX_WORKSPACE_SIZE = 1 << 30

# Model
MODEL_FILE = 'models/lanenet_trt.pb'
INPUT_NAME = 'input_tensor'
INPUT_SHAPE = (3, 256, 512)
OUTPUT_NAME = 'lanenet_model/vgg_backend/binary_seg/SoftMax'

def build_engine(model_file):
    print('build engine...')

    def get_frozen_graph(graph_file):
        """Read Frozen Graph file from disk."""
        with tf.gfile.FastGFile(graph_file, "rb") as f:
            graph_def = tf.GraphDef()
            graph_def.ParseFromString(f.read())
        return graph_def

    # The TensorRT inference graph
    trt_graph = get_frozen_graph(model_file)

    # print([n.name for n in trt_graph.node])

    input_names = ['image']

    # Create session and load graph
    tf_config = tf.ConfigProto()
    tf_config.gpu_options.allow_growth = True
    tf_config.gpu_options.per_process_gpu_memory_fraction=0.1

    tf_sess = tf.Session(config=tf_config)
    tf.import_graph_def(trt_graph, name='')

    tf_input = tf_sess.graph.get_tensor_by_name('input_tensor:0')
    tf_output = tf_sess.graph.get_tensor_by_name('lanenet_model/vgg_backend/binary_seg/ArgMax:0')

    return trt_graph, tf_input, tf_output, tf_sess


def load_input(image):
    image = cv2.resize(image, (512, 256), interpolation=cv2.INTER_LINEAR)
    image = image / 127.5 - 1.0
    return image


def do_inference(n, context, h_input, d_input, h_output, d_output):
    # Transfer input data to the GPU.
    cuda.memcpy_htod(d_input, h_input)

    # Run inference.
    st = time.time()
    context.execute(batch_size=1, bindings=[int(d_input), int(d_output)])
    print('Inference time {}: {} [msec]'.format(n, (time.time() - st)*1000))

    # Transfer predictions back from the GPU.
    cuda.memcpy_dtoh(h_output, d_output)
    return h_output


def parse_args():
    parser = argparse.ArgumentParser(description='TensorTRT execution sample')
    parser.add_argument('video', help='input video')
    
    return parser.parse_args()


trt_graph, tf_input, tf_output, tf_sess = build_engine("models/lanenet_trt.pb")

def inference(img):
    global trt_graph, tf_input, tf_output, tf_sess

    img = cv2.resize(img, (512, 256), interpolation=cv2.INTER_LINEAR)
    img = load_input(img)

    pred = tf_sess.run([tf_output], feed_dict={
        tf_input: image[None, ...]
    })
    # print(pred)
    mask = pred[0][0]
    mask *= 255
    mask = mask.astype(np.uint8)

    return mask
