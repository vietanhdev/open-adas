#if !defined(COLLISION_WARNING_H)
#define COLLISION_WARNING_H


#include <QCloseEvent>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <condition_variable>
#include "traffic_object.h"
#include "camera_model.h"
#include "car_status.h"

class CollisionWarning {

    std::mutex mtx;
    std::condition_variable cv;
    bool new_data_available = false;

    cv::Mat img;
    std::vector<TrafficObject> objects;
    std::shared_ptr<CameraModel> camera_model;
    std::shared_ptr<CarStatus> car_status;

   public:

    CollisionWarning(std::shared_ptr<CameraModel> camera_model, std::shared_ptr<CarStatus> car_status);

    static void processingThread(CollisionWarning* this_ptr);

    void updateData(const cv::Mat &img, const std::vector<TrafficObject> &objects);
    void calculateDistance(const cv::Mat &img, std::vector<TrafficObject> &objects);
};

#endif  // COLLISION_WARNING_H
