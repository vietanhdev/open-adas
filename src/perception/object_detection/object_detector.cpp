#include "object_detector.h"

using namespace std;

ObjectDetector::ObjectDetector() {

    // If plan file is existed => Load it
    if (fs::exists(SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN)) {

        cout << "Loading TensorRT plan file at: " << SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN << endl;
        net = new ctdet::ctdetNet(SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN);

    } else { // Else, create engine file

        cout << "TensorRT plan file not found. Creating a new plan file at: " << SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN << endl;
        if (SMARTCAM_OBJECT_DETECTION_MODE == std::string("FLOAT32")) {
            net = new ctdet::ctdetNet(SMARTCAM_OBJECT_DETECTION_MODEL, "", ctdet::RUN_MODE::FLOAT32);
        } else if (SMARTCAM_OBJECT_DETECTION_MODE == std::string("FLOAT16")) {
            net = new ctdet::ctdetNet(SMARTCAM_OBJECT_DETECTION_MODEL, "", ctdet::RUN_MODE::FLOAT16);
        } else {
            cout << "TensorRT mode " << SMARTCAM_OBJECT_DETECTION_MODE << " is not supported now. Please build model using `build_tensorrt_engine`" << endl;
        }
        net->saveEngine(SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN);

    }
    
    outputData = std::unique_ptr<float[]>(new float[net->outputBufferSize]);
}

std::vector<TrafficObject> ObjectDetector::detect(const cv::Mat &img, const cv::Mat &original_img) {

    cv::Mat frame(img);
    auto inputData = prepareImage(frame, net->forwardFace);

    net->doInference(inputData.data(), outputData.get());

    int num_det = static_cast<int>(outputData[0]);

    std::vector<Detection> detected_objects;
    detected_objects.resize(num_det);
    memcpy(detected_objects.data(), &outputData[1], num_det * sizeof(Detection));

    postProcess(detected_objects, img, net->forwardFace);

    // Do traffic sign classification
    float fx = static_cast<float>(original_img.cols) / img.cols;
    float fy = static_cast<float>(original_img.rows) / img.rows;
    std::vector<TrafficObject> traffic_objects;
    int original_img_height = original_img.rows;
    int original_img_width = original_img.cols;

    // Filter by size
    std::vector<Detection> filtered_detected_object(detected_objects.size());
    auto it = std::copy_if (detected_objects.begin(), detected_objects.end(), filtered_detected_object.begin(), [](Detection d){
        return d.bbox.x2 - d.bbox.x1 >= MIN_OBJECT_SIZE &&
        d.bbox.y2 - d.bbox.y1 >= MIN_OBJECT_SIZE;
    } );
    filtered_detected_object.resize(std::distance(filtered_detected_object.begin(),it));  // shrink container to new size
    detected_objects = filtered_detected_object;

    // Classify traffic signs
    std::vector<size_t> sign_object_ids;
    std::vector<cv::Mat> sign_crops;
    for (size_t i = 0; i < detected_objects.size(); ++i) {

        if (detected_objects[i].classId == 8) { // Traffic sign

            int x1 = min(original_img_width - 1, static_cast<int>(fx * detected_objects[i].bbox.x1));
            int x2 = min(original_img_width - 1, static_cast<int>(fx * detected_objects[i].bbox.x2));
            int y1 = min(original_img_height - 1, static_cast<int>(fy * detected_objects[i].bbox.y1));
            int y2 = min(original_img_height - 1, static_cast<int>(fy * detected_objects[i].bbox.y2));
            int width = x2 - x1;
            int height = y2 - y1;

            if (width > MIN_TRAFFIC_SIGN_SIZE && height > MIN_TRAFFIC_SIGN_SIZE) {

                cv::Rect roi(x1, y1, width, height);
                cv::Mat crop = original_img(roi);

                sign_crops.push_back(crop);
                sign_object_ids.push_back(i);

            }
            
        }
        
        traffic_objects.push_back(
            TrafficObject(detected_objects[i], "")
        );
    }

    std::vector<std::string> sign_names = sign_classifier.getSignNames(sign_crops);
    for (size_t i = 0; i < sign_object_ids.size(); ++i) {
        traffic_objects[sign_object_ids[i]].traffic_sign_type = sign_names[i];
    }

    return traffic_objects;
}

void ObjectDetector::drawDetections(const std::vector<TrafficObject> & result,cv::Mat& img)
{

    int box_think = (img.rows+img.cols) * .001 ;
    float label_scale = 0.75;
    int base_line ;
    for (const auto &item : result) {
        std::string label;
        std::stringstream stream;
        std::string class_name = ctdet::className[item.classId];

        if (!isInStrVector(class_name, ctdet::drawClassNames)) {
            continue;
        }
        

        if (item.distance_to_my_car != -1)
            stream << std::fixed << std::setprecision(1) << item.distance_to_my_car;

        std::getline(stream,label);

        auto size = cv::getTextSize(label,cv::FONT_HERSHEY_PLAIN,label_scale,1,&base_line);

        cv::rectangle(img, cv::Point(item.bbox.x1,item.bbox.y1),
                      cv::Point(item.bbox.x2 ,item.bbox.y2),
                      cv::Scalar(0,255,0), box_think*2, 8, 0);
        
        cv::putText(img,label,
                cv::Point(item.bbox.x1, item.bbox.y2 - size.height),
                cv::FONT_HERSHEY_PLAIN, label_scale , cv::Scalar(0,0,255), box_think/1.2, 8, 0);

    }
}


bool ObjectDetector::isInStrVector(const std::string &value, const std::vector<std::string> &array) {
    return std::find(array.begin(), array.end(), value) != array.end();
}