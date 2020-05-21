#include "camera_model.h"
#include "config.h"

using namespace std;
using namespace cv;

CameraModel::CameraModel(std::shared_ptr<CarStatus> car_status) {

    this->car_status = car_status;
    this->camera_wizard = std::make_unique<CameraWizard>(this->car_status);
    this->camera_wizard->setStyleSheet("QAbstractButton { height: 50px }");

    // Prevent resize error when access simulation first
    this->camera_wizard->showFullScreen();
    this->camera_wizard->hide();

    connect(this->camera_wizard.get(), SIGNAL(updateCameraModel(float, float, float, float, float, float, float, float, float, float, float, float)), this, SLOT(updateCameraModel(float, float, float, float, float, float, float, float, float, float, float, float)));

    readCalibFile(SMARTCAM_CAMERA_CALIB_FILE);

}

void CameraModel::showCameraWizard() {
    this->camera_wizard->restart();
    this->camera_wizard->showFullScreen();
}


void CameraModel::readCalibFile(std::string file_path) {

    if (!fs::exists(file_path)) {
        return;
    }

    // Read data file
    std::ifstream data_file(file_path);

    float car_width; float carpet_width; 
    float car_to_carpet_distance; float carpet_length;
    float tl_x; float tl_y;
    float tr_x; float tr_y;
    float br_x; float br_y;
    float bl_x; float bl_y;

    std::string line;
    data_file >> line >> car_width;
    data_file >> line >> carpet_width;
    data_file >> line >> car_to_carpet_distance;
    data_file >> line >> carpet_length;
    data_file >> line >> tl_x >> line >> tl_y;
    data_file >> line >> tr_x >> line >> tr_y;
    data_file >> line >> br_x >> line >> br_y;
    data_file >> line >> bl_x >> line >> bl_y;

    updateCameraModel(car_width, carpet_width, car_to_carpet_distance, carpet_length,
        tl_x, tl_y, tr_x, tr_y, br_x, br_y, bl_x, bl_y);
}

void CameraModel::updateCameraModel(
    float car_width, float carpet_width, 
    float car_to_carpet_distance, float carpet_length,
    float tl_x, float tl_y,
    float tr_x, float tr_y,
    float br_x, float br_y,
    float bl_x, float bl_y
) {

    cout << "car_width " << car_width << endl;
    cout << "carpet_width " << carpet_width << endl;
    cout << "car_to_carpet_distance " << car_to_carpet_distance << endl;
    cout << "carpet_length " << carpet_length << endl;
    cout << "tl_x " << tl_x << endl;
    cout << "tl_y " << tl_y << endl;
    cout << "tr_x " << tr_x << endl;
    cout << "tr_y " << tr_y << endl;
    cout << "br_x " << br_x << endl;
    cout << "br_y " << br_y << endl;
    cout << "bl_x " << bl_x << endl;
    cout << "bl_y " << bl_y << endl;
    
    FourPoints four_image_points = FourPoints(
        cv::Point2f(tl_x, tl_y),
        cv::Point2f(tr_x, tr_y),
        cv::Point2f(br_x, br_y),
        cv::Point2f(bl_x, bl_y)
    );
    birdview_model.calibrate(car_width, carpet_width, car_to_carpet_distance, carpet_length, four_image_points);
}

BirdViewModel *CameraModel::getBirdViewModel() {
    return &birdview_model;
}