#include "traffic_sign_images.h"

using namespace std;

TrafficSignImages::TrafficSignImages() {

    // Read images
    for (size_t i = 0; i < valid_speeds.size(); ++i) {

        if (valid_speeds[i] == 0) {
            images.push_back(TrafficSignImage(valid_speeds[i], 
            IMG_DIR + "END_OF_SPEED_LIMIT.png", DEFAULT_IMG_PATH));
        }

        images.push_back(TrafficSignImage(valid_speeds[i], 
            IMG_DIR + "SPEED_LIMIT_" + std::to_string(valid_speeds[i]) + ".png", DEFAULT_IMG_PATH));
    }

}

cv::Mat TrafficSignImages::getSpeedSignImage(int speed) {

    for (size_t i = 0; i < images.size(); ++i) {
        if (speed == images[i].speed) {
            return images[i].image.clone();
        }
    }

    return cv::Mat();
}
