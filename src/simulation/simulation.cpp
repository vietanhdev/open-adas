#include "simulation.h"
#include "config.h"
#include "ui_simulation.h"

using namespace std;
using namespace cv;


void Simulation::setupAndConnectComponents() {
    setupUi(this);

    // Setup virtual CAN
    if (USE_CAN_BUS_FOR_SIMULATION_DATA) {
        system("sh setup_vcan.sh");
    }

    // Connect buttons
    connect(this->playBtn, SIGNAL(released()), this, SLOT(playBtnClicked()));
    connect(this->simDataList, SIGNAL(itemSelectionChanged()), this,
            SLOT(simDataList_onselectionchange()));
    connect(this->closeBtn, SIGNAL(released()), this, SLOT(closeBtnClicked()));

    connect(this->softRestartBtn, SIGNAL(released()), this, SLOT(softRestart()));
    connect(this->shutdownBtn, SIGNAL(released()), this, SLOT(shutdown()));
    

    // Read simulation data
    std::ifstream sim_list_file(SMARTCAM_SIMULATION_LIST);

    std::string line;
    std::getline(sim_list_file, line);
    int n_simulations = std::stoi(line);

    for (size_t i = 0; i < n_simulations; ++i) {
        std::string video_path;
        std::string data_path;
        std::getline(sim_list_file, video_path);
        std::getline(sim_list_file, data_path);

        sim_data.push_back(SimData(video_path, data_path));

        std::string video_name = fs::path(video_path).filename();
        QString simulation_name_qs = QString::fromUtf8(video_name.c_str());
        QListWidgetItem *new_sim_data = new QListWidgetItem(
            QIcon(":/resources/images/play.png"),
            simulation_name_qs);
        new_sim_data->setData(Qt::UserRole, QVariant(static_cast<int>(i)));
        this->simDataList->addItem(new_sim_data);
    }

}

Simulation::Simulation(std::shared_ptr<CarStatus> car_status, std::shared_ptr<CameraModel> camera_model, QWidget *parent)
    : QWidget(parent) {
    setupAndConnectComponents();
    this->car_status = car_status;
    this->camera_model = camera_model;
}


Simulation::Simulation(std::shared_ptr<CarStatus> car_status, std::string input_video_path, std::string input_data_path, QWidget *parent): QWidget(parent)  {
    setupAndConnectComponents();
    setVideoPath(input_video_path);
    setDataFilePath(input_data_path);
    this->car_status = car_status;
}

void Simulation::softRestart() {
    system("pkill CarSmartCam; ./CarSmartCam &");
}

void Simulation::shutdown() {
    system("sudo shutdown now");
}


int Simulation::readSimulationData(std::string video_path, std::string data_file_path, SimulationData &sim_data) {

    sim_data = SimulationData();

    cout << video_path << endl;

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

            // Skip the first line
            std::getline(data_file, line);

            if (sim_data.begin_frame < 0) {
                sim_data.begin_frame = 0;
            }
            if (sim_data.end_frame < 0) {
                sim_data.end_frame = n_frames - 1;
            }
            if (sim_data.playing_fps < 0) {
                sim_data.playing_fps = fps;
            }

            sim_data.sim_frames.reserve(sim_data.end_frame + 1);

            std::string line;
            while (std::getline(data_file, line)) {
                if (line != "---") { // End of this block
                    int begin_frame;
                    int end_frame;
                    float speed;
                    int turning_left, turning_right;
                    std::istringstream iss(line);
                    iss >> begin_frame >> end_frame >> speed >> turning_left >> turning_right;
                    sim_data.sim_frames.push_back(
                        SimFrameData(begin_frame, end_frame, speed, turning_left, turning_right));

                    assert(begin_frame <= end_frame);

                    for (size_t i = begin_frame; i <= end_frame; ++i) {
                        sim_data.sim_frames[i].car_speed = speed;
                        sim_data.sim_frames[i].turning_left = turning_left;
                        sim_data.sim_frames[i].turning_right = turning_right;
                    }
                    
                } else {
                    break;
                }
            }
        } else if (line == "CameraCalibration") {

            float car_width; float carpet_width; 
            float car_to_carpet_distance; float carpet_length;
            float tl_x; float tl_y;
            float tr_x; float tr_y;
            float br_x; float br_y;
            float bl_x; float bl_y;

            std::string line;
            data_file >> line >> car_width;
            data_file >> line >> carpet_width;
            data_file >> line >> car_to_carpet_distance;
            data_file >> line >> carpet_length;
            data_file >> line >> tl_x >> line >> tl_y;
            data_file >> line >> tr_x >> line >> tr_y;
            data_file >> line >> br_x >> line >> br_y;
            data_file >> line >> bl_x >> line >> bl_y;

            camera_model->updateCameraModel(
                car_width, carpet_width, car_to_carpet_distance, carpet_length,
                tl_x, tl_y, tr_x, tr_y, br_x, br_y, bl_x, bl_y);

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

    this_ptr->playing_thread_running = true;

    std::string video_path = this_ptr->getVideoPath();
    std::string data_file_path = this_ptr->getDataFilePath();

    SimulationData sim_data;
    int read_success = this_ptr->readSimulationData(video_path, data_file_path, sim_data);

    if (read_success != 0) {
        this_ptr->playing_thread_running = false;
        return;
    }

    cv::Mat frame;
    size_t current_frame_id = 0;

    // Set begin frame
    if (sim_data.begin_frame != 0) {
        current_frame_id = sim_data.begin_frame;
        sim_data.capture.set(cv::CAP_PROP_POS_FRAMES, sim_data.begin_frame);
    }

    // Reset car status
    this_ptr->car_status->reset();

    while (this_ptr->isPlaying()) {
        
        if (current_frame_id > sim_data.end_frame) {
            break;
        }

        this_ptr->setCarStatus(sim_data.sim_frames[current_frame_id].car_speed,
                sim_data.sim_frames[current_frame_id].turning_left,
                sim_data.sim_frames[current_frame_id].turning_right
            );
        
        this_ptr->playing_thread_running = true;
        sim_data.capture >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        this_ptr->car_status->setCurrentImage(frame);

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
    }

    // Load selected simulation
    if (selected_sim_data_indices.empty()) {
        QMessageBox::critical(
        this, "Notification",
        "Please choose simulation data first.");
    } else {
        int selected_sim_data_id = selected_sim_data_indices[0];
        cout << "VIDEO PATH: " << sim_data[selected_sim_data_id].video_path << endl;
        cout << "DATA PATH: " << sim_data[selected_sim_data_id].data_path << endl;
        setVideoPath(sim_data[selected_sim_data_id].video_path);
        setDataFilePath(sim_data[selected_sim_data_id].data_path);

        startPlaying();
        this->hide();
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

        // IMPORTANT!!! Wait for playing thread to stop
        while (playing_thread_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

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
}

void Simulation::setCarSpeed(float speed) {
    car_speed = speed;
    if (!USE_CAN_BUS_FOR_SIMULATION_DATA) {
        car_status->setCarSpeed(speed);
    } else {
        can_bus_emitter.sendSpeed(speed);
    }
}

void Simulation::setCarStatus(float speed, bool turning_left, bool turning_right) {
    if (!USE_CAN_BUS_FOR_SIMULATION_DATA) {
        car_status->setCarStatus(speed, turning_left, turning_right);
    } else {
        can_bus_emitter.sendSpeed(speed);
        can_bus_emitter.sendTurnSignal(turning_left, turning_right);
    }
}

void Simulation::simDataList_onselectionchange() {
    QList<QListWidgetItem *> selected_sim_data = this->simDataList->selectedItems();

    // Save selected simulation data
    selected_sim_data_indices.clear();
    for (int i = 0; i < selected_sim_data.count(); ++i) {
        selected_sim_data_indices.push_back(
            selected_sim_data[i]->data(Qt::UserRole).toInt());
    }
}


void Simulation::closeBtnClicked() {
    this->hide();
}
