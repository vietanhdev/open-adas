#include "four_point_select_page.h"

FourPointSelectPage::FourPointSelectPage(QWidget *parent, CarStatus *car_status) :
    QWizardPage(parent),
    ui(new Ui::FourPointSelectPage)
{
    ui->setupUi(this);
    this->car_status = car_status;
    getNewImage();

    connect(ui->getImageBtn, SIGNAL(released()), this, SLOT(getNewImage()));
 
    // Fields
    // registerField("carWidthInput", ui->carWidthInput);
    // registerField("carpetWidthInput", ui->carpetWidthInput);
    // registerField("carToCarpetDistanceInput", ui->carToCarpetDistanceInput);
    // registerField("carpetLengthInput", ui->carpetLengthInput);

}


void FourPointSelectPage::getNewImageFromCam() {
   setImage(car_status->getCurrentImage());
}

void FourPointSelectPage::setImage(const cv::Mat &image) {
    std::lock_guard<std::mutex> guard(image_mutex);
    this->image = image;
}

cv::Mat FourPointSelectPage::getImage() {
    std::lock_guard<std::mutex> guard(image_mutex);
    return image;
}