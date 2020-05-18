#include "camera_model.h"
#include "config.h"
#include "ui_camera_model.h"

using namespace std;
using namespace cv;


void CameraModel::setupAndConnectComponents() {
    setupUi(this);

    // Connect buttons
    // connect(this->playBtn, SIGNAL(released()), this, SLOT(playBtnClicked()));
    // connect(this->simDataList, SIGNAL(itemSelectionChanged()), this,
    //         SLOT(simDataList_onselectionchange()));
    // connect(this->closeBtn, SIGNAL(released()), this, SLOT(closeBtnClicked()));
    
}

CameraModel::CameraModel(CarStatus *car_status, QWidget *parent)
    : QWidget(parent) {
    setupAndConnectComponents();
}

void CameraModel::closeBtnClicked() {
    this->hide();
}
