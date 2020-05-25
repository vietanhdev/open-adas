#include "sign_classifier.h"

int main(int argc, char** argv)
{

    TrafficSignClassifier classifier;
    cv::Mat input_img = cv::imread(argv[1]);

    std::vector<cv::Mat> input_imgs({input_img});
    std::vector<int> sign_id = classifier.getSignIds(input_imgs);
    cout << "Sign ID: " << sign_id[0] << endl;

    std::vector<std::string> sign_name = classifier.getSignNames(input_imgs);
    cout << "Sign name: " << sign_name[0] << endl;

    return 0;
}