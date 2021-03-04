#ifndef MEASUREMENT_PAGE_H
#define MEASUREMENT_PAGE_H

#include <QFileDialog>
#include <QWizardPage>
#include "ui_measurement_page.h"

class MeasurementPage : public QWizardPage {
    Q_OBJECT
   private:
    Ui::MeasurementPage *ui;
   public:
    MeasurementPage(QWidget*);
};

#endif