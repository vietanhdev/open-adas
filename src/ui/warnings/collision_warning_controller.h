#if !defined(COLLISION_WARNING_CONTROLLER_H)
#define COLLISION_WARNING_CONTROLLER_H

#include <mutex>
#include <opencv2/opencv.hpp>
#include <condition_variable>
#include "perception/object_detection/traffic_object.h"
#include "perception/camera_model/camera_model.h"
#include "sensors/car_status.h"

class CollisionWarningController {

    std::mutex mtx;
    std::condition_variable cv;
    bool new_data_available = false;

    cv::Mat img;
    std::vector<TrafficObject> objects;
    std::shared_ptr<CameraModel> camera_model;
    std::shared_ptr<CarStatus> car_status;

   public:

    CollisionWarningController(std::shared_ptr<CameraModel> camera_model, std::shared_ptr<CarStatus> car_status);

    static void processingThread(CollisionWarningController* this_ptr);

    void updateData(const cv::Mat &img, const std::vector<TrafficObject> &objects);
    void calculateDistance(const cv::Mat &img, std::vector<TrafficObject> &objects);
    bool isInDangerSituation(const cv::Size &img_size,        
    std::vector<TrafficObject> &objects);
};

#endif  // COLLISION_WARNING_CONTROLLER_H
