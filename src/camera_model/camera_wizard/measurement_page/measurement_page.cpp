#include "measurement_page.h"

MeasurementPage::MeasurementPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MeasurementPage)
{
    ui->setupUi(this);
 
    // Fields
    registerField("carWidth", ui->carWidthInput, "value", SIGNAL(valueChanged(double)));
    registerField("carpetWidth", ui->carpetWidthInput, "value", SIGNAL(valueChanged(double)));
    registerField("carToCarpetDistance", ui->carToCarpetDistanceInput, "value", SIGNAL(valueChanged(double)));
    registerField("carpetLength", ui->carpetLengthInput, "value", SIGNAL(valueChanged(double)));

}