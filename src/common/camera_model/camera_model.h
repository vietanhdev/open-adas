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

#include "camera_wizard.h"
#include "car_status.h"
#include "config.h"
#include "filesystem_include.h"
#include "birdview_model.h"
#include "utils.h"

class CameraModel : public QObject {
    Q_OBJECT
    std::shared_ptr<CarStatus> car_status;
    std::unique_ptr<CameraWizard> camera_wizard;
    BirdViewModel birdview_model;

   public:
    explicit CameraModel(std::shared_ptr<CarStatus> car_status);
    void showCameraWizard();
    void readCalibFile(std::string file_path);
    BirdViewModel *getBirdViewModel();

   public slots:
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
