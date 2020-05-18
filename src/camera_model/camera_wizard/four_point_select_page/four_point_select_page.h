#ifndef FOUR_POINT_SELECT_PAGE_H
#define FOUR_POINT_SELECT_PAGE_H

#include <QFileDialog>
#include <QWizardPage>
#include "ui_four_point_select_page.h"

class FourPointSelectPage : public QWizardPage {
   private:
    Ui::FourPointSelectPage *ui;
   public:
    FourPointSelectPage(QWidget*);
};

#endif