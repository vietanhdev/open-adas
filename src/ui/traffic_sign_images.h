#ifndef TRAFFIC_SIGN_IMAGES_H
#define TRAFFIC_SIGN_IMAGES_H
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

struct TrafficSignImage {
    static constexpr int kImgWidth = 48;
    static constexpr int kImgHeight = 48;
    int speed;
    cv::Mat image;
    TrafficSignImage(int speed, std::string img_path, std::string default_img_path): speed(speed) {
        std::cout << img_path << std::endl;
        
        image = cv::imread(img_path);
        if (image.empty()) {
            image = cv::imread(default_img_path);
        }
        cv::resize(image, image, cv::Size(kImgWidth, kImgHeight));
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