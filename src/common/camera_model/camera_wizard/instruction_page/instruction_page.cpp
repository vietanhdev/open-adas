#include "instruction_page.h"

InstructionPage::InstructionPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::InstructionPage)
{
    ui->setupUi(this);
}