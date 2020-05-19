#ifndef CAMERA_WIZARD_H
#define CAMERA_WIZARD_H

#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QtWidgets>
#include "instruction_page/instruction_page.h"
#include "measurement_page/measurement_page.h"
#include "four_point_select_page/four_point_select_page.h"
#include "car_status.h"

class CameraWizard : public QWizard {
    Q_OBJECT
    CarStatus *car_status;

   public:
    CameraWizard(CarStatus *car_status);

   private slots:
    void onFinishButtonClicked();

};

#endif
