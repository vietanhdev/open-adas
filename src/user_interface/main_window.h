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
#include "config.h"
#include "file_storage.h"
#include "lane_detector.h"
#include "object_detector.h"
#include "car_gps_reader.h"
#include "input_source.h"
#include "simulation.h"
#include "car_status.h"
#include "timer.h"

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

    QGraphicsPixmapItem pixmap;

    // Processors
    std::shared_ptr<ObjectDetector> object_detector;
    std::shared_ptr<LaneDetector> lane_detector;
    std::shared_ptr<CarGPSReader> car_gps_reader;

   public:
    CarStatus car_status;

   private:

    static void objectDetectionThread(
        std::shared_ptr<ObjectDetector> object_detector, CarStatus *);
    static void laneDetectionThread(
        std::shared_ptr<LaneDetector> lane_detector, CarStatus *);
    static void carPropReaderThread(
        std::shared_ptr<CarGPSReader> car_gps_reader);

   public:
    void setInputSource(InputSource input_source);
    void setSimulation(Simulation *simulation);

};

#endif  // MAINWINDOW_H
