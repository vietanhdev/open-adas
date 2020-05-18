#ifndef CAMERA_MODEL_H
#define CAMERA_MODEL_H

#include <QCloseEvent>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <QListWidgetItem>
#include <algorithm>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <regex>
#include <sstream>
#include <thread>
#include <condition_variable>
#include <QWidget>
#include <QFileDialog>
#include <chrono>
#include <fstream>
#include <string>
#include <bits/stdc++.h> 
#include "config.h"
#include "ui_camera_model.h"
#include "camera_model_data.h"
#include "car_status.h"
#include "filesystem_include.h"


class CameraModel : public QWidget, private Ui::CameraModel {
    Q_OBJECT

   private slots:
    void closeBtnClicked();

   public:
    explicit CameraModel(CarStatus *car_status, QWidget *parent = nullptr);
    void setupAndConnectComponents();
   
};

#endif
