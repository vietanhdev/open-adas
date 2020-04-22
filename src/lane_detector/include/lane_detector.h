#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <python2.7/Python.h>
#include <opencv2/opencv.hpp>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <iostream>


namespace py = boost::python;
namespace np = boost::python::numpy;

class LaneDetector {
   private:
     py::object main_module;
     py::object mn;
     py::object image_processor;
     py::object process_img;
   public:

     bool ready = false;

    LaneDetector();
    
    void init_python();
    void init();

    cv::Mat detect_lane(const cv::Mat& img);

    // Function to convert from cv::Mat to numpy array
    np::ndarray ConvertMatToNDArray(const cv::Mat& mat);

    // Function to convert from numpy array to cv::Mat
    cv::Mat ConvertNDArrayToMat(const np::ndarray& ndarr);
};

#endif