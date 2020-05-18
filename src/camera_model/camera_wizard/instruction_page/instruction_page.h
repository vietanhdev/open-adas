#ifndef INSTRUCTION_PAGE_H
#define INSTRUCTION_PAGE_H

#include <QFileDialog>
#include <QWizardPage>
#include "ui_instruction_page.h"

class InstructionPage : public QWizardPage {
   private:
    Ui::InstructionPage *ui;
   public:
    InstructionPage(QWidget*);
};

#endif