#include "camera_wizard.h"


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
    this->car_status = car_status;
    this->addPage(createIntroPage());
    this->addPage(new InstructionPage(nullptr));
    this->addPage(new MeasurementPage(nullptr));
    this->addPage(new FourPointSelectPage(nullptr, car_status));
    this->addPage(createFinishPage());
    this->setWindowTitle("Camera Calibration Wizard");
}