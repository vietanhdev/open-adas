#include "lane_detector.h"


using namespace cv;

void LaneDetector::init_python() {
    // Set your python location.
    // wchar_t str[] = L"/home/vietanhdev/miniconda3/envs/example_env";
    // Py_SetPythonHome(str);

    setenv("PYTHONPATH", "./python_lane_detector", 1);

    Py_Initialize();
    np::initialize();
}

LaneDetector::LaneDetector() {}


void LaneDetector::init() {
    try {
        // Initialize boost python and numpy
        init_python();

        // Import module
        main_module = py::import("__main__");

        // Load the dictionary for the namespace
        mn = main_module.attr("__dict__");

        // Import the ConfigParser module into the namespace
        py::exec("import init_lane_detector", mn);

        // Create the locally-held RawConfigParser object
        image_processor = py::eval("init_lane_detector.get_lane_finder()", mn);

        // Get processing method
        this->process_img = image_processor.attr("get_lane_mask");

        this->ready = true;

    } catch (py::error_already_set&) {
        PyErr_Print();
        exit(1);
    }
}

cv::Mat LaneDetector::detect_lane(const cv::Mat& img) {

    try {

        cv::Mat clone_img = img.clone();
        np::ndarray nd_img = ConvertMatToNDArray(clone_img);
        np::ndarray output_img = py::extract<np::ndarray>(process_img(nd_img));
        cv::Mat mat_img = ConvertNDArrayToMat(output_img);
        
        return mat_img;

    } catch (py::error_already_set&) {
        PyErr_Print();
        exit(1);
    }

    return cv::Mat();

}

// Function to convert from cv::Mat to numpy array
np::ndarray LaneDetector::ConvertMatToNDArray(const cv::Mat& mat) {
    py::tuple shape = py::make_tuple(mat.rows, mat.cols, mat.channels());
    py::tuple stride =
        py::make_tuple(mat.channels() * mat.cols * sizeof(uchar),
                    mat.channels() * sizeof(uchar), sizeof(uchar));
    np::dtype dt = np::dtype::get_builtin<uchar>();
    np::ndarray ndImg =
        np::from_data(mat.data, dt, shape, stride, py::object());

    return ndImg;
}


// Function to convert from numpy array to cv::Mat
cv::Mat LaneDetector::ConvertNDArrayToMat(const np::ndarray& ndarr) {
    int length =
        ndarr.get_nd();  // get_nd() returns num of dimensions. this is used as
                        // a length, but we don't need to use in this case.
                        // because we know that image has 3 dimensions.
    const Py_intptr_t* shape =
        ndarr.get_shape();  // get_shape() returns Py_intptr_t* which we can get
                            // the size of n-th dimension of the ndarray.
    char* dtype_str = py::extract<char*>(py::str(ndarr.get_dtype()));

    // Variables for creating Mat object
    int rows = shape[0];
    int cols = shape[1];
    int channel = length == 3 ? shape[2] : 1;
    int depth;

    // Find corresponding datatype in C++
    if (!strcmp(dtype_str, "uint8")) {
        depth = CV_8U;
    } else if (!strcmp(dtype_str, "int8")) {
        depth = CV_8S;
    } else if (!strcmp(dtype_str, "uint16")) {
        depth = CV_16U;
    } else if (!strcmp(dtype_str, "int16")) {
        depth = CV_16S;
    } else if (!strcmp(dtype_str, "int32")) {
        depth = CV_32S;
    } else if (!strcmp(dtype_str, "float32")) {
        depth = CV_32F;
    } else if (!strcmp(dtype_str, "float64")) {
        depth = CV_64F;
    } else {
        std::cout << "Wrong dtype error" << std::endl;
        return cv::Mat();
    }

    int type = CV_MAKETYPE(
        depth, channel);  // Create specific datatype using channel information

    cv::Mat mat = cv::Mat(rows, cols, type);
    memcpy(mat.data, ndarr.get_data(), sizeof(uchar) * rows * cols * channel);

    return mat;
}
