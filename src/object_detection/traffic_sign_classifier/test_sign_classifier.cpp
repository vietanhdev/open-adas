#include <opencv2/opencv.hpp>
#include "sign_classifier.h"
#include "filesystem_include.h"
#include <fstream>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{

    const std::string keys =
        "{help h usage ? |      | print this message   }"
        "{input_folder   |      | input image folder }"
        "{output_file    |test_result_TensorRT.txt| path to output file }"
        ;

    CommandLineParser parser(argc, argv, keys);
    parser.about("Traffic sign classifier testing tool");
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string input_folder = parser.get<std::string>("input_folder");
    std::string output_file = parser.get<std::string>("output_file");

    ofstream logfile;
    logfile.open (output_file);
    TrafficSignClassifier classifier;
    using namespace std::chrono; 
    double total_time = 0;
    size_t n_images = 0;
    for(auto& p: fs::recursive_directory_iterator(input_folder)) {
        if(p.path().extension() == ".png" || p.path().extension() == ".jpg") {
            n_images += 1;
            std::string img_path = p.path().string();
            cout << n_images << " " << img_path << endl;
            cv::Mat input_img = cv::imread(img_path);
            std::vector<cv::Mat> input_imgs({input_img});
            auto start = high_resolution_clock::now(); 
            std::vector<int> sign_id = classifier.getSignIds(input_imgs);
            auto stop = high_resolution_clock::now(); 
            auto duration = duration_cast<microseconds>(stop - start);
            long long int execution_time = duration.count();
            double milliseconds = 0.001 * execution_time;
            total_time += milliseconds;
            logfile << img_path << " " << sign_id[0] << endl;
        }
    }
    cout << "Avg. time: " << total_time / n_images << " ms" << endl;
    logfile.close();

    return 0;
}