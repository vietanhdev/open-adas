#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <SDL.h>

#include <QCloseEvent>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <algorithm>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <regex>
#include <sstream>
#include <thread>

#include "utility.h"
#include "camera.h"
#include "car_prop_reader.h"
#include "config.h"
#include "file_storage.h"
#include "lane_detector.h"
#include "object_detector.h"
#include "object_detector_with_tracking.h"
#include "input_source.h"
#include "simulation.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void startVideoGrabber();
    void refreshCams();

   protected:
    void closeEvent(QCloseEvent *event);
    void loadEffects();

   private slots:
    void changeCamClicked();

   private:
    Ui::MainWindow *ui;

    // Input source
    InputSource input_source;
    Simulation *simulation;

    // Current image
    cv::Mat current_img;
    std::mutex current_img_mutex;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;

    // Camera to use
    std::vector<struct Camera> available_cams;
    int current_camera_index = 0;

    std::shared_ptr<ObjectDetector> object_detector;
    std::shared_ptr<ObjectDetectorWithTracking> object_detector_with_tracking;
    std::shared_ptr<LaneDetector> lane_detector;
    std::shared_ptr<CarPropReader> car_prop_reader;
    std::unique_ptr<CTracker> object_tracker;

    std::vector<Detection> object_detection_results;
    std::mutex object_detection_results_mutex;

    std::vector<TrackingObject> object_tracking_results;
    std::mutex object_tracking_results_mutex;

    cv::Mat lane_line_mask;
    cv::Mat detected_line_img;
    cv::Mat reduced_line_img;
    std::vector<LaneLine> lane_detection_results;
    std::mutex lane_detection_results_mutex;

   public:
    void setCurrentImage(const cv::Mat &img);
    cv::Mat getCurrentImage();

   private:
    std::string getInputVideoPath();

    static void objectTrackingThread(
        std::shared_ptr<ObjectDetectorWithTracking> object_detector,
        cv::Mat &img, std::mutex &img_mutex,
        std::vector<TrackingObject> &object_tracking_results,
        std::mutex &object_tracking_results_mutex);
    static void laneDetectionThread(
        std::shared_ptr<LaneDetector> lane_detector, MainWindow *);
    static void carPropReaderThread(
        std::shared_ptr<CarPropReader> car_prop_reader);

   public:
    void setInputSource(InputSource input_source);
    void setSimulation(Simulation *simulation);
};

#endif  // MAINWINDOW_H
