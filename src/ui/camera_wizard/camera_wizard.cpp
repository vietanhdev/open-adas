#include "camera_wizard.h"

using namespace std;

QWizardPage *createIntroPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Warning!!!");

    QLabel *label = new QLabel("This wizard will help you calibrate your camera.\n"
                               "Camera calibration should only be done in a competent garage with sufficient instrumentation.\n"
                               "You should only continue when you know exactly what you are doing.\n");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}


QWizardPage *createFinishPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Finished");

    QLabel *label = new QLabel("Camera calibration finished.");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}


CameraWizard::CameraWizard(std::shared_ptr<CarStatus> car_status) {
    measurement_page = std::make_shared<MeasurementPage>(nullptr);
    this->car_status = car_status;
    this->addPage(createIntroPage());
    this->addPage(new InstructionPage(nullptr));
    this->addPage(measurement_page.get());
    this->addPage(new FourPointSelectPage(nullptr, car_status));
    this->addPage(createFinishPage());
    this->setWindowTitle("Camera Calibration Wizard");
    connect(this->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(onFinishButtonClicked()));
}


void CameraWizard::onFinishButtonClicked() {

    float car_width = field("car_width").toFloat();
    float carpet_width = field("carpet_width").toFloat();
    float car_to_carpet_distance = field("car_to_carpet_distance").toFloat();
    float carpet_length = field("carpet_length").toFloat();
    float tl_x = field("tl_x").toFloat();
    float tl_y = field("tl_y").toFloat();
    float tr_x = field("tr_x").toFloat();
    float tr_y = field("tr_y").toFloat();
    float br_x = field("br_x").toFloat();
    float br_y = field("br_y").toFloat();
    float bl_x = field("bl_x").toFloat();
    float bl_y = field("bl_y").toFloat();

    // Write new data to file
    if( remove( SMARTCAM_CAMERA_CALIB_FILE ) != 0 )
        cout << "Error deleting file" << endl;
    std::ofstream calib_file;
    calib_file.open (SMARTCAM_CAMERA_CALIB_FILE);
    calib_file << "car_width " << car_width << endl;
    calib_file << "carpet_width " << carpet_width << endl;
    calib_file << "car_to_carpet_distance " << car_to_carpet_distance << endl;
    calib_file << "carpet_length " << carpet_length << endl;
    calib_file << "tl_x " << tl_x << endl;
    calib_file << "tl_y " << tl_y << endl;
    calib_file << "tr_x " << tr_x << endl;
    calib_file << "tr_y " << tr_y << endl;
    calib_file << "br_x " << br_x << endl;
    calib_file << "br_y " << br_y << endl;
    calib_file << "bl_x " << bl_x << endl;
    calib_file << "bl_y " << bl_y << endl;
    calib_file.close();

    emit updateCameraModel(
        car_width, carpet_width, car_to_carpet_distance, carpet_length,
        tl_x, tl_y, tr_x, tr_y, br_x, br_y, bl_x, bl_y
    );
   
}