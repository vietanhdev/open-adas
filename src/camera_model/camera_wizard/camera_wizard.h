#ifndef CAMERA_WIZARD_H
#define CAMERA_WIZARD_H

#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QtWidgets>
#include "instruction_page/instruction_page.h"
#include "measurement_page/measurement_page.h"
#include "four_point_select_page/four_point_select_page.h"

class CameraWizard : public QWizard {
    Q_OBJECT

   public:
    CameraWizard();
};

#endif
