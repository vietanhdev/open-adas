#include "sign_classifier.h"

int main(int argc, char** argv)
{

    TrafficSignClassifier classifier;
    cv::Mat input_img = cv::imread(argv[1]);

    int sign_id = classifier.getSignId(input_img);
    cout << "Sign ID: " << sign_id << endl;

    std::string sign_name = classifier.getSignName(input_img);
    cout << "Sign name: " << sign_name << endl;

    return 0;
}