#include "measurement_page.h"

MeasurementPage::MeasurementPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MeasurementPage)
{
    ui->setupUi(this);
 
    // Fields
    registerField("carWidthInput", ui->carWidthInput);
    registerField("carpetWidthInput", ui->carpetWidthInput);
    registerField("carToCarpetDistanceInput", ui->carToCarpetDistanceInput);
    registerField("carpetLengthInput", ui->carpetLengthInput);

}