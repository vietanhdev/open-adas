#ifndef TRAFFIC_SIGN_IMAGES_H
#define TRAFFIC_SIGN_IMAGES_H
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

struct TrafficSignImage {
    const int IMG_WIDTH = 64;
    const int IMG_HEIGHT = 64;
    int speed;
    cv::Mat image;
    TrafficSignImage(int speed, std::string img_path, std::string default_img_path): speed(speed) {
        std::cout << img_path << std::endl;
        
        image = cv::imread(img_path);
        if (image.empty()) {
            image = cv::imread(default_img_path);
        }
        cv::resize(image, image, cv::Size(IMG_WIDTH, IMG_HEIGHT));
    }
};

class TrafficSignImages {
   private:
    const std::string IMG_DIR = "images/traffic_signs/";
    const std::vector<int> valid_speeds = 
    {0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
    const std::string DEFAULT_IMG_PATH = "images/traffic_signs/DEFAULT.png";
    std::vector<TrafficSignImage> images;
    
   public:
    TrafficSignImages();
    cv::Mat getSpeedSignImage(int speed);
};


#endif