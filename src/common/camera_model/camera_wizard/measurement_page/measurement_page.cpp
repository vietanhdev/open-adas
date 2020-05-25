#include "measurement_page.h"

MeasurementPage::MeasurementPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MeasurementPage)
{
    ui->setupUi(this);
 
    // Fields
    registerField("car_width", ui->carWidthInput, "value", SIGNAL(valueChanged(double)));
    registerField("carpet_width", ui->carpetWidthInput, "value", SIGNAL(valueChanged(double)));
    registerField("car_to_carpet_distance", ui->carToCarpetDistanceInput, "value", SIGNAL(valueChanged(double)));
    registerField("carpet_length", ui->carpetLengthInput, "value", SIGNAL(valueChanged(double)));

}