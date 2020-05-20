#if !defined(COLLISION_WARNING_H)
#define COLLISION_WARNING_H


#include <QCloseEvent>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <condition_variable>
#include "traffic_object.h"
#include "camera_model.h"

class CollisionWarning {

    std::mutex mtx;
    std::condition_variable cv;
    bool new_data_avaiable = false;

    cv::Mat img;
    std::vector<TrafficObject> objects;
    std::shared_ptr<CameraModel> camera_model;

   public:

    CollisionWarning(std::shared_ptr<CameraModel> camera_model);

    static void processingThread(CollisionWarning* this_ptr);

    void updateData(const cv::Mat &img, const std::vector<TrafficObject> &objects);
};

#endif  // COLLISION_WARNING_H
