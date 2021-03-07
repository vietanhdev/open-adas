#ifndef FOUR_POINTS_H
#define FOUR_POINTS_H

#include <opencv2/opencv.hpp>

class FourPoints {
   public:
    cv::Point2f tl;
    cv::Point2f tr;
    cv::Point2f br;
    cv::Point2f bl;

    FourPoints() {}

    FourPoints(cv::Point2f tl, cv::Point2f tr, cv::Point2f br, cv::Point2f bl)
        : tl(tl), tr(tr), br(br), bl(bl) {}

    std::vector<cv::Point2f> to_vector() {
        return std::vector<cv::Point2f>({tl, tr, br, bl});
    }

    std::vector<cv::Point2f> to_vector(float scale_x, float scale_y) {
        return std::vector<cv::Point2f>({
            cv::Point2f(tl.x * scale_x, tl.y * scale_y), 
            cv::Point2f(tr.x * scale_x, tr.y * scale_y), 
            cv::Point2f(br.x * scale_x, br.y * scale_y),  
            cv::Point2f(bl.x * scale_x, bl.y * scale_y)});
    }
};

#endif  // FOUR_POINTS_H
