#ifndef SIMULATION_H
#define SIMULATION_H

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
#include <condition_variable>
#include <QWidget>
#include <QFileDialog>
#include <chrono>
#include <fstream>
#include <string>
#include <bits/stdc++.h> 
#include "config.h"
#include "ui_simulation.h"
#include "simulation_data.h"

class Simulation : public QWidget, private Ui::Simulation {
    Q_OBJECT

   private:
    std::atomic<float> car_speed = 0.0;
    std::atomic<bool> is_playing = false;
    std::atomic<bool> playing_thread_running = false;
    std::mutex path_mutex;
    std::string video_path;
    std::string data_file_path;
    std::mutex current_img_mutex;
    cv::Mat current_img;

   public:
    explicit Simulation(QWidget *parent = nullptr);
    explicit Simulation(std::string input_video_path, std::string input_data_path, QWidget *parent = nullptr);
    void setupAndConnectComponents();
    void stopPlaying();
    void startPlaying();
    cv::Mat getCurrentImage();
    void setCarSpeed(float);
    float getCarSpeed();

   private slots:
    void selectVideoBtnClicked();
    void selectDataFileBtnClicked();
    void playBtnClicked();

   private:
    static void playingThread(Simulation * this_ptr);
    void setVideoPath(std::string);
    std::string getVideoPath();
    void setDataFilePath(std::string);
    std::string getDataFilePath();
    bool isPlaying();
    void setPlaying(bool playing);
    void setCurrentImage(const cv::Mat &img);

    int readSimulationData(std::string video_path, std::string data_file_path, SimulationData &sim_data);
   
};

#endif  // MAINWINDOW_H
