#include "camera_model.h"
#include "config.h"

using namespace std;
using namespace cv;

CameraModel::CameraModel(CarStatus *car_status) {
    this->car_status = car_status;
    this->camera_wizard = std::make_unique<CameraWizard>(this->car_status);
    this->camera_wizard->setStyleSheet("QAbstractButton { height: 50px }");
}

void CameraModel::showCameraWizard() {
    this->camera_wizard->showFullScreen();
}
