#ifndef CAMERA_MODEL_H
#define CAMERA_MODEL_H

#include <bits/stdc++.h>

#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <QWidget>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#include "camera_model_data.h"
#include "camera_wizard.h"
#include "car_status.h"
#include "config.h"
#include "filesystem_include.h"

class CameraModel {

    CarStatus *car_status;
    std::unique_ptr<CameraWizard> camera_wizard;

   public:
    explicit CameraModel(CarStatus *car_status);
    void showCameraWizard();
};

#endif
