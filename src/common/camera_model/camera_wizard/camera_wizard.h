#ifndef CAMERA_WIZARD_H
#define CAMERA_WIZARD_H

#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QtWidgets>
#include <stdio.h>
#include <fstream>
#include "instruction_page/instruction_page.h"
#include "measurement_page/measurement_page.h"
#include "four_point_select_page/four_point_select_page.h"
#include "car_status.h"

class CameraWizard : public QWizard {
    Q_OBJECT
    std::shared_ptr<CarStatus> car_status;
    std::shared_ptr<MeasurementPage> measurement_page;

   public:
    CameraWizard(std::shared_ptr<CarStatus> car_status);

   private slots:
    void onFinishButtonClicked();

   signals:
    void updateCameraModel(
        float car_width, float carpet_width, 
        float car_to_carpet_distance, float carpet_length,
        float tl_x, float tl_y,
        float tr_x, float tr_y,
        float br_x, float br_y,
        float bl_x, float bl_y
    );

};

#endif
