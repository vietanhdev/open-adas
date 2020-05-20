#include "collision_warning.h"

CollisionWarning::CollisionWarning(std::shared_ptr<CameraModel> camera_model) {
    this->camera_model = camera_model;
}

void CollisionWarning::processingThread(CollisionWarning *this_ptr) {
    cv::Mat img;
    std::vector<TrafficObject> objects;
    while (true) {
        {
            std::unique_lock<std::mutex> lck(this_ptr->mtx);
            while (!this_ptr->new_data_avaiable) this_ptr->cv.wait(lck);
            this_ptr->img.copyTo(img);
            objects = this_ptr->objects;
            this_ptr->new_data_avaiable = false;
        }

        cv::Mat transform_img = this_ptr->camera_model->getBirdViewModel()->transformImage(img);
        cv::imwrite("debug_tranform.png", transform_img);

    }
}

void CollisionWarning::updateData(const cv::Mat &img, const std::vector<TrafficObject> &objects) {
    std::unique_lock<std::mutex> lck(mtx);
    this->img = img.clone();
    this->objects = objects;
    new_data_avaiable = true;
    cv.notify_all();
}
