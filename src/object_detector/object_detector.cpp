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

std::vector<TrafficObject> ObjectDetector::detect(const cv::Mat &img) {

    cv::Mat frame(img);
    auto inputData = prepareImage(frame, net->forwardFace);

    net->doInference(inputData.data(), outputData.get());

    int num_det = static_cast<int>(outputData[0]);

    std::vector<Detection> detected_objects;
    detected_objects.resize(num_det);
    memcpy(detected_objects.data(), &outputData[1], num_det * sizeof(Detection));

    postProcess(detected_objects, img, net->forwardFace);

    // Do traffic sign classification
    std::vector<TrafficObject> traffic_objects;
    for (size_t i = 0; i < detected_objects.size(); ++i) {
        std::string extended_type = "";
        if (detected_objects[i].classId == 8) { // Traffic sign
            cv::Rect roi(
                cv::Point(detected_objects[i].bbox.x1, detected_objects[i].bbox.y1),
                cv::Point(detected_objects[i].bbox.x2, detected_objects[i].bbox.y2));
            cv::Mat crop = img(roi);

            std::string sign_name = sign_classifier.getSignName(crop);
            extended_type = sign_name;

            if (extended_type != "OTHER") {
                cout << extended_type << endl;
            }
            
        }
        
        traffic_objects.push_back(
            TrafficObject(detected_objects[i], extended_type)
        );
    }

    return traffic_objects;
}

void ObjectDetector::drawDetections(const std::vector<TrafficObject> & result,cv::Mat& img)
{

    int box_think = (img.rows+img.cols) * .001 ;
    float label_scale = img.rows * 0.0009;
    int base_line ;
    for (const auto &item : result) {
        std::string label;
        std::stringstream stream;
        stream << ctdet::className[item.classId] << ":" << item.extended_type  << " " << item.prob << std::endl;
        std::getline(stream,label);

        auto size = cv::getTextSize(label,cv::FONT_HERSHEY_COMPLEX,label_scale,1,&base_line);

        cv::rectangle(img, cv::Point(item.bbox.x1,item.bbox.y1),
                      cv::Point(item.bbox.x2 ,item.bbox.y2),
                      cv::Scalar(0,255,0), box_think*2, 8, 0);
        
        cv::putText(img,label,
                cv::Point(item.bbox.x2,item.bbox.y2 - size.height),
                cv::FONT_HERSHEY_COMPLEX, label_scale , cv::Scalar(0,0,255), box_think/2, 8, 0);

    }
}