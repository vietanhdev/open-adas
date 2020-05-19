#include "camera_model.h"
#include "config.h"

using namespace std;
using namespace cv;

CameraModel::CameraModel(CarStatus *car_status) {
    this->car_status = car_status;
    this->camera_wizard = std::make_unique<CameraWizard>(this->car_status);
    this->camera_wizard->setStyleSheet("QAbstractButton { height: 50px }");

    // Prevent resize error when access simulation first
    this->camera_wizard->showFullScreen();
    this->camera_wizard->hide();

}

void CameraModel::showCameraWizard() {
    this->camera_wizard->restart();
    this->camera_wizard->showFullScreen();
}
