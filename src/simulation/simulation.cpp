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


int Simulation::readSimulationData(std::string video_path, std::string data_file_path, SimulationData &sim_data) {

    sim_data = SimulationData();

    // Open video
    if (!sim_data.capture.open(video_path)) {
        QMessageBox::critical(
            NULL, "Video path",
            "Could not open video file");
        return 1;
    }

    // Get FPS
    float fps = sim_data.capture.get(cv::CAP_PROP_FPS);
    int n_frames = sim_data.capture.get(cv::CAP_PROP_FRAME_COUNT);
    cout << "Number of video frames: " << n_frames << endl;

    // Read data file
    std::ifstream data_file(data_file_path);

    std::string line;
    while (std::getline(data_file, line)) {

        if (line == "VideoProps") { // Video props
            std::string line;
            while (std::getline(data_file, line)) {
                if (line != "---") { // End of this block
                    std::string prop_name;
                    std::string prop_value;
                    std::istringstream iss(line);
                    iss >> prop_name >> prop_value;
                    if (prop_name == "playing_speed") {
                        sim_data.playing_fps = std::stof(prop_value);
                    } else if (prop_name == "begin_frame") {
                        sim_data.begin_frame = std::stoi(prop_value);
                    } else if (prop_name == "end_frame") {
                        sim_data.end_frame = std::stoi(prop_value);
                    }
                } else {
                    break;
                }
            } 
        } else if (line == "CarSpeed") {

            if (sim_data.begin_frame < 0) {
                sim_data.begin_frame = 0;
            }
            if (sim_data.end_frame < 0) {
                sim_data.end_frame = n_frames - 1;
            }
            if (sim_data.playing_fps < 0) {
                sim_data.playing_fps = fps;
            }

            sim_data.frame_to_speed.reserve(sim_data.end_frame + 1);

            std::string line;
            while (std::getline(data_file, line)) {
                if (line != "---") { // End of this block
                    int begin_frame;
                    int end_frame;
                    float speed;
                    std::istringstream iss(line);
                    iss >> begin_frame >> end_frame >> speed;
                    sim_data.speed_data.push_back(
                        SpeedData(begin_frame, end_frame, speed));

                    for (int i = begin_frame; i <= end_frame; ++i) {{
                        sim_data.frame_to_speed[i] = speed;
                    }}
                    
                } else {
                    break;
                }
            }
        }

        if (sim_data.begin_frame < 0) {
            sim_data.begin_frame = 0;
        }
        if (sim_data.end_frame < 0) {
            sim_data.end_frame = n_frames - 1;
        }
        if (sim_data.playing_fps < 0) {
            sim_data.playing_fps = fps;
        }

    }

    return 0;

} 


void Simulation::playingThread(Simulation * this_ptr) {

    std::string video_path = this_ptr->getVideoPath();
    std::string data_file_path = this_ptr->getDataFilePath();

    SimulationData sim_data;
    int read_success = this_ptr->readSimulationData(video_path, data_file_path, sim_data);

    if (read_success != 0) {
        this_ptr->playing_thread_running = false;
        return;
    }

    this_ptr->fpsLabel->setText(QString::number(sim_data.playing_fps));
    
    cv::Mat frame;
    size_t current_frame_id = 0;

    // Set begin frame
    if (sim_data.begin_frame != 0) {
        current_frame_id = sim_data.begin_frame;
        sim_data.capture.set(cv::CAP_PROP_POS_FRAMES, sim_data.begin_frame);
    }

    while (this_ptr->isPlaying()) {
        
        if (current_frame_id > sim_data.end_frame) {
            break;
        }

        this_ptr->setCarSpeed(sim_data.frame_to_speed[current_frame_id]);
        
        this_ptr->playing_thread_running = true;
        sim_data.capture >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        this_ptr->setCurrentImage(frame);

        std::this_thread::sleep_for(std::chrono::microseconds(int(1.0 / sim_data.playing_fps * 1e6)));

        ++current_frame_id;
    }

    sim_data.capture.release();
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
    simDataPathLabel->setText(QString::fromStdString(data_file_path));
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

void Simulation::setCarSpeed(float speed) {
    car_speed = speed;
    speedLabel->setText(QString::number(speed));
}

float Simulation::getCarSpeed() {
    return car_speed;
}