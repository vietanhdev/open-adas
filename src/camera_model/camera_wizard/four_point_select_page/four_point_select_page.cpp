#include "four_point_select_page.h"

FourPointSelectPage::FourPointSelectPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::FourPointSelectPage)
{
    ui->setupUi(this);
 
    // Fields
    // registerField("carWidthInput", ui->carWidthInput);
    // registerField("carpetWidthInput", ui->carpetWidthInput);
    // registerField("carToCarpetDistanceInput", ui->carToCarpetDistanceInput);
    // registerField("carpetLengthInput", ui->carpetLengthInput);

}