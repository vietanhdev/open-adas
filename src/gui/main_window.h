#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <QShortcut>

#include <SDL.h>

#include <algorithm>
#include <thread>
#include <mutex>
#include <memory>
#include "opencv2/opencv.hpp"

#include "file_storage.h"

#include "object_detector.h"
#include "lane_detector.h"
#include "object_detector.h"
#include "ctdet_utils.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showCam();

protected:
    void closeEvent(QCloseEvent *event);
    void loadEffects();

private slots:
    void changeCam_clicked();
    void refreshCams();
    
private:
    Ui::MainWindow *ui;

    // Current image
    // When user click "Capture", we take photo here then have it
    // to [Photos] folder
    cv::Mat current_img;
    std::mutex current_img_mutex;

    // File Storage
    ml_cam::FileStorage fs;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;

    // Camera to use
    int MAX_CAMS = 5; // Max number of camera supported. This number used to scan cameras
    int current_camera_index = 0;
    int selected_camera_index = 0;

    ObjectDetector *object_detector;
    LaneDetector *lane_detector;

    std::vector<Detection> object_detection_results;
    std::mutex object_detection_results_mutex;

public:
    void setCurrentImage(const cv::Mat & img);
    cv::Mat getCurrentImage();

    static void object_detection_thread(ObjectDetector * object_detector, cv::Mat & img, std::mutex & img_mutex, 
    std::vector<Detection> & object_detection_results, std::mutex & object_detection_results_mutex);
};

#endif // MAINWINDOW_H
