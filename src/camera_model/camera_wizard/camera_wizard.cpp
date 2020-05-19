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


CameraWizard::CameraWizard(CarStatus *car_status) {
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

    cout << "carWidth " << field("carWidth").toFloat() << endl;
    cout << "carpetWidth " << field("carpetWidth").toFloat() << endl;
    cout << "carToCarpetDistance " << field("carToCarpetDistance").toFloat() << endl;
    cout << "carpetLength " << field("carpetLength").toFloat() << endl;
    cout << "tl_x " << field("tl_x").toFloat() << endl;
    cout << "tl_y " << field("tl_y").toFloat() << endl;
    cout << "tr_x " << field("tr_x").toFloat() << endl;
    cout << "tr_y " << field("tr_y").toFloat() << endl;
    cout << "br_x " << field("br_x").toFloat() << endl;
    cout << "br_y " << field("br_y").toFloat() << endl;
    cout << "bl_x " << field("bl_x").toFloat() << endl;
    cout << "bl_y " << field("bl_y").toFloat() << endl;
   
}