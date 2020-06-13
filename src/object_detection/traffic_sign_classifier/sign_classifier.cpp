#include "sign_classifier.h"

#include "config.h"

using namespace cv;

TrafficSignClassifier::TrafficSignClassifier() {

    params.inputW = SIGN_CLASSIFICATION_INPUT_WIDTH;
    params.inputH = SIGN_CLASSIFICATION_INPUT_HEIGHT;
    params.batchSize = SIGN_CLASSIFICATION_BATCH_SIZE;
    params.nClasses = SIGN_CLASSIFICATION_N_CLASSES;
    params.uffFilePath = SIGN_CLASSIFICATION_MODEL;
    params.engineFilePath = SIGN_CLASSIFICATION_TENSORRT_PLAN;
    params.classListFile = SIGN_CLASSIFICATION_CLASS_LIST;
    params.forceRebuildEngine = SIGN_CLASSIFICATION_FORCE_REBUILD_ENGINE;
    params.inputTensorNames.push_back(SIGN_CLASSIFICATION_INPUT_NODE);
    params.outputTensorNames.push_back(SIGN_CLASSIFICATION_OUTPUT_NODE);
    params.fp16 = SIGN_CLASSIFICATION_USE_FP_16;

    auto test = gLogger.defineTest("TrafficSignClassifier", 0, NULL);
    gLogger.reportTestStart(test);

    model = std::make_shared<ClassificationNet>(params);
}

std::vector<int> TrafficSignClassifier::getSignIds(const std::vector<cv::Mat>& input_imgs) {

    std::vector<int> labels;
    if (input_imgs.empty()) return labels;

    int n_images = input_imgs.size();
    int n_batchs = ceil(static_cast<float>(n_images) / params.batchSize);
    for (int batch = 0; batch < n_batchs; ++batch) {

        std::vector<int> batch_labels;
        int begin_id = batch * params.batchSize;
        int end_id = batch * params.batchSize + params.batchSize;
        if (end_id > n_images) {
            end_id = batch * params.batchSize + n_images - batch * params.batchSize;
        }
        std::vector<cv::Mat>::const_iterator first = input_imgs.begin() + begin_id;
        std::vector<cv::Mat>::const_iterator last = input_imgs.begin() + end_id;
        std::vector<cv::Mat> inputs(first, last);

        if (!model->infer(inputs, batch_labels, SIGN_CLASSIFICATION_THRESH)) {
            cerr << "Error on running traffic sign classification model." << endl;
        }

        labels.reserve(labels.size() + distance(batch_labels.begin(),batch_labels.end()));
        labels.insert(labels.end(), batch_labels.begin(), batch_labels.end());

    }

    assert(input_imgs.size() == labels.size());


    if (!input_imgs.empty() && DEBUG_WRITE_SIGN_CROPS) {
        fs::create_directory("debug_traffic_sign");
        for (size_t i = 0; i < input_imgs.size(); i++)
        {
            std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(p);
            cv::imwrite("debug_traffic_sign/" + std::string(std::ctime(&t)) + std::to_string(i) + ".png", input_imgs[i]);
        }   
    }

    return labels;
}

std::vector<std::string> TrafficSignClassifier::getSignNames(const std::vector<cv::Mat>& input_imgs) {
    std::vector<int> ids = getSignIds(input_imgs);
    return getSignNames(ids);
}

std::vector<std::string> TrafficSignClassifier::getSignNames(std::vector<int>& class_ids) {
    std::vector<std::string> sign_names;
    for (size_t i = 0; i < class_ids.size(); ++i) {
        sign_names.push_back(model->getClassName(class_ids[i]));
    }
    return sign_names;
}

std::string TrafficSignClassifier::getSignName(int class_id) {
    return model->getClassName(class_id);
}

bool TrafficSignClassifier::isSpeedSign(std::string sign_name) {
    if (sign_name.find("SPEED_LIMIT") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
