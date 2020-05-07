#include "simulation.h"
#include "config.h"
#include "ui_simulation.h"

using namespace std;
using namespace cv;


void Simulation::setupAndConnectComponents() {
    setupUi(this);

    // Connect buttons
    connect(this->selectVideoBtn, SIGNAL(released()), this, SLOT(selectVideoBtnClicked()));
    connect(this->selectDataFileBtn, SIGNAL(released()), this, SLOT(selectDataFileBtnClicked()));
    connect(this->playBtn, SIGNAL(released()), this, SLOT(playBtnClicked()));
}

Simulation::Simulation(QWidget *parent)
    : QWidget(parent) {
    setupAndConnectComponents();
}


Simulation::Simulation(std::string input_video_path, std::string input_data_path, QWidget *parent): QWidget(parent)  {
    setupAndConnectComponents();
    setVideoPath(input_video_path);
    setDataFilePath(input_data_path);
}


void Simulation::playingThread(Simulation * this_ptr) {

    std::string video_path = this_ptr->getVideoPath();
    std::string data_file_path = this_ptr->getDataFilePath();

    VideoCapture cap;

    if (!cap.open(video_path)) {
        QMessageBox::critical(
            this_ptr, "Video path",
            "Could not open video file");
        return;
    }

    // Get FPS
    double fps = cap.get(cv::CAP_PROP_FPS);
    this_ptr->fpsLabel->setText(QString::number(fps));
    
    cv::Mat frame;
    while (this_ptr->isPlaying()) {

        this_ptr->playing_thread_running = true;
        cap >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        this_ptr->setCurrentImage(frame);

        // TODO: Add wait
    }

    this_ptr->playing_thread_running = false;

}

void Simulation::selectVideoBtnClicked() {

    stopPlaying();

    QString video_file = QFileDialog::getOpenFileName(this,
    tr("Open Video File"), "", tr("Video Files (*.avi *.mp4 *.mov)"));

    setVideoPath(video_file.toUtf8().constData());

}

void Simulation::selectDataFileBtnClicked() {

    stopPlaying();

    QString video_file = QFileDialog::getOpenFileName(this,
    tr("Open Data File"), "", tr("Camera Data Files (*.txt)"));

    setDataFilePath(video_file.toUtf8().constData());

}

void Simulation::playBtnClicked() {

    if (isPlaying()) {
        stopPlaying();
    } else {
        startPlaying();
    }

}

void Simulation::startPlaying() {

    if (getVideoPath() == "") {
        QMessageBox::critical(0,"Video path missing","Please select a video !");
        return;
    }

    if (getDataFilePath() == "") {
        QMessageBox::critical(0,"Data file path missing","Please select a data file !");
        return;
    }

    if (isPlaying()) {
        setPlaying(false);
    }

    // Wait for playing thread to stop
    while (playing_thread_running);

    // Start new playing thread
    setPlaying(true);
    std::thread playing_thread(Simulation::playingThread, this);
    playing_thread.detach(); 

}

void Simulation::stopPlaying() {
    if (isPlaying()) {
        setPlaying(false);
    }
}

void Simulation::setVideoPath(std::string path) {
    std::lock_guard<std::mutex> guard(path_mutex);
    video_path = path;
    videoPathLabel->setText(QString::fromStdString(video_path));
}

std::string Simulation::getVideoPath() {
    std::lock_guard<std::mutex> guard(path_mutex);
    return video_path;
}

void Simulation::setDataFilePath(std::string path) {
    std::lock_guard<std::mutex> guard(path_mutex);
    data_file_path = path;
    simDataPathLabel->setText(QString::fromStdString(    data_file_path));
}

std::string Simulation::getDataFilePath() {
    std::lock_guard<std::mutex> guard(path_mutex);
    return data_file_path;
}


bool Simulation::isPlaying() {
    return is_playing;
}

void Simulation::setPlaying(bool playing) {
    is_playing = playing;

    if (is_playing) {
        playBtn->setText("Stop");
    } else {
        playBtn->setText("Play");
    }
}

cv::Mat Simulation::getCurrentImage() {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.clone();
}

void Simulation::setCurrentImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    current_img = img.clone();
}