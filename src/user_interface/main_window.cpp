#include "main_window.h"
#include "ui_main_window.h"
#include "config.h"

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Connect buttons
    connect(ui->simulationBtn, SIGNAL(released()), this, SLOT(openSimulationSelector()));
    connect(ui->muteBtn, SIGNAL(released()), this, SLOT(toggleMute()));
    connect(ui->alertBtn, SIGNAL(released()), this, SLOT(toggleAlert()));
    connect(ui->setupBtn, SIGNAL(released()), this, SLOT(showCameraWizard()));

    car_status = std::make_shared<CarStatus>();
    camera_model = std::make_shared<CameraModel>(car_status);
    object_detector = std::make_shared<ObjectDetector>();
    lane_detector = std::make_shared<LaneDetector>();
    car_gps_reader = std::make_shared<CarGPSReader>();
    collision_warning = std::make_shared<CollisionWarning>(camera_model, car_status);

    // Start processing threads
    std::thread od_thread(&MainWindow::objectDetectionThread, 
        object_detector,
        car_status,
        collision_warning.get()
        );
    od_thread.detach();

#ifndef DISABLE_LANE_DETECTOR
    std::thread ld_thread(&MainWindow::laneDetectionThread, 
        lane_detector,
        car_status);
    ld_thread.detach();
#endif

    std::thread cpr_thread(&MainWindow::carPropReaderThread,
                           car_gps_reader);
    cpr_thread.detach();

    std::thread speed_warning_thread(&MainWindow::speedWarningThread, car_status, this);
    speed_warning_thread.detach();

    // std::thread collision_warning_thread(&CollisionWarning::processingThread, collision_warning.get());
    // collision_warning_thread.detach();

}

void MainWindow::speedWarningThread(std::shared_ptr<CarStatus> car_status, MainWindow *main_window) {

    while (true) {

        MaxSpeedLimit speed_limit = car_status->getMaxSpeedLimit();
        main_window->setSpeedLimit(speed_limit);

        // Play sound
        if (speed_limit.speed_limit >= 0 &&
            !speed_limit.has_notified) {
            if (speed_limit.speed_limit > 0) {
                main_window->playAudio("traffic_signs/" + std::to_string(speed_limit.speed_limit) + ".wav");
            } else {
                main_window->playAudio("traffic_signs/00.wav");
            }
        }

        if (speed_limit.overspeed_warning &&
            !speed_limit.overspeed_warning_has_notified) {
            main_window->alert("traffic_signs/warning_overspeed.wav");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    
}

void MainWindow::objectDetectionThread(
    std::shared_ptr<ObjectDetector> object_detector,
    std::shared_ptr<CarStatus> car_status,
    CollisionWarning *collision_warning) {
        
    cv::Mat image;
    cv::Mat original_image;

    Timer::time_point_t car_status_start_time = car_status->getStartTime();
    TrafficSignMonitor traffic_sign_monitor(car_status);

    while (true) {

        // Reset traffic sign monitor if car status has changed
        // (In case of changing simulation)
        if (car_status_start_time != car_status->getStartTime()) {
            cout << "CarStatus has been reset!" << endl;
            car_status_start_time = car_status->getStartTime();
            traffic_sign_monitor = TrafficSignMonitor(car_status);
        }

        car_status->getCurrentImage(image, original_image);

        if (image.empty()) {
            continue;
        }

        Timer::time_point_t begin_time = Timer::getCurrentTime();
        std::vector<TrafficObject> objects = object_detector->detect(image, original_image);
        car_status->setObjectDetectionTime(Timer::calcTimePassed(begin_time));
        traffic_sign_monitor.updateTrafficSign(objects);

        if (SHOW_DISTANCES) {
            collision_warning->calculateDistance(image, objects);
        }   
        
        car_status->setDetectedObjects(objects);

    }
}

void MainWindow::laneDetectionThread(
    std::shared_ptr<LaneDetector> lane_detector, std::shared_ptr<CarStatus> car_status) {
    cv::Mat clone_img;
    while (true) {

        car_status->getCurrentImage(clone_img);
        if (clone_img.empty()) {
            continue;
        }

        #if defined (DEBUG_LANE_DETECTOR_SHOW_LINES)  || defined (DEBUG_LANE_DETECTOR_SHOW_LINE_MASK)
        cv::Mat lane_line_mask;
        cv::Mat detected_line_img;
        cv::Mat reduced_line_img;

        Timer::time_point_t begin_time = Timer::getCurrentTime();
        std::vector<LaneLine> detected_lines = lane_detector->detectLaneLines(clone_img, lane_line_mask, detected_line_img, reduced_line_img);
        car_status->setLaneDetectionTime(Timer::calcTimePassed(begin_time));
        car_status->setDetectedLaneLines(detected_lines, lane_line_mask, detected_line_img, reduced_line_img);
        #else
        Timer::time_point_t begin_time = Timer::getCurrentTime();
        std::vector<LaneLine> detected_lines = lane_detector->detectLaneLines(clone_img);
        car_status->setLaneDetectionTime(Timer::calcTimePassed(begin_time));
        car_status->setDetectedLaneLines(detected_lines);
        #endif 
    }
}

void MainWindow::carPropReaderThread(
    std::shared_ptr<CarGPSReader> car_gps_reader) {
    while (true) {
        car_gps_reader->updateProps();
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::playAudio(std::string audio_file) {
    if (!is_mute)
        system(("canberra-gtk-play -f sounds/" + audio_file + " &").c_str());
}

void MainWindow::alert(std::string audio_file) {
    if (is_alert)
        playAudio(audio_file);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QApplication::quit();
    exit(0);
}

void MainWindow::startVideoGrabber() {

    Mat draw_frame;
    Timer::time_point_t last_fps_show = Timer::getCurrentTime();
    Timer::time_duration_t object_detection_time =
         car_status->getObjectDetectionTime();
    Timer::time_duration_t lane_detection_time =
         car_status->getLaneDetectionTime();
    while (true) {

        // Processing
        car_status->getCurrentImage(draw_frame);

        if (!draw_frame.empty()) {

            std::vector<LaneLine> detected_lane_lines = car_status->getDetectedLaneLines();
                
            if (!detected_lane_lines.empty()) {

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINE_MASK
                cv::Mat lane_line_mask_copy = car_status->getLineMask();
                #endif

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINES
                cv::Mat detected_line_img_copy = car_status->getDetectedLinesViz();
                cv::Mat reduced_line_img_copy = car_status->getReducedLinesViz();
                #endif

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINE_MASK
                    if (!lane_line_mask_copy.empty()) {
                        cv::resize(lane_line_mask_copy, lane_line_mask_copy, draw_frame.size());

                        cv::Mat rgb_lane_result =
                            cv::Mat::zeros(draw_frame.size(), CV_8UC3);

                        rgb_lane_result.setTo(Scalar(0, 0, 255), lane_line_mask_copy > 0.5);
                        draw_frame.setTo(Scalar(0, 0, 0), lane_line_mask_copy > 0.5);

                        cv::addWeighted(draw_frame, 1, rgb_lane_result, 1, 0,
                                        draw_frame);
                    }
                #endif

                #ifdef DEBUG_SHOW_FPS

                if (Timer::calcTimePassed(last_fps_show) > 1000) {
                    object_detection_time =
                        car_status->getObjectDetectionTime();
                    lane_detection_time =
                        car_status->getLaneDetectionTime();
                    last_fps_show = Timer::getCurrentTime();
                }
                
                #endif

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINES
                    if (!detected_line_img_copy.empty()) {
                        cv::namedWindow("Detected Lines", cv::WINDOW_NORMAL);
                        cv::imshow("Detected Lines", detected_line_img_copy);
                        cv::waitKey(1);
                    }
                    if (!reduced_line_img_copy.empty()) {
                        cv::namedWindow("Reduced Lines", cv::WINDOW_NORMAL);
                        cv::imshow("Reduced Lines", reduced_line_img_copy);
                        cv::waitKey(1);
                    }
                #endif
                
            }

            float danger_distance = car_status->getCarSpeed() / 3.6 * 1.5;
            cv::Mat danger_zone = camera_model->getBirdViewModel()->getDangerZone(draw_frame.size(), danger_distance);
            cv::Mat rgb_danger_zone = cv::Mat::zeros(draw_frame.size(), CV_8UC3);
            rgb_danger_zone.setTo(Scalar(0, 0, 255), danger_zone > 0.5);
            cv::addWeighted(draw_frame, 1, rgb_danger_zone, 0.3, 0,
                                    draw_frame);

            cv::putText(draw_frame, "Object detection: " +  std::to_string(object_detection_time) + " ms", Point2f(10,10), FONT_HERSHEY_PLAIN, 0.8,  Scalar(0,0,255,255), 1.5);
            cv::putText(draw_frame, "Lane detection: " + std::to_string(lane_detection_time) + " ms", Point2f(10,20), FONT_HERSHEY_PLAIN, 0.8,  Scalar(0,0,255,255), 1.5);

            std::vector<TrafficObject> detected_objects = car_status->getDetectedObjects();

            if (!detected_objects.empty()) {
                object_detector->drawDetections(
                    detected_objects, draw_frame);
            }

            // Show speed sign
            MaxSpeedLimit speed_limit = getSpeedLimit();
            if (speed_limit.speed_limit > 0) {
                ml_cam::place_overlay(draw_frame, traffic_sign_images.getSpeedSignImage(speed_limit.speed_limit), 32, 32);
            }
    
            // ### Show current image
            QImage qimg(draw_frame.data, static_cast<int>(draw_frame.cols),
                        static_cast<int>(draw_frame.rows),
                        static_cast<int>(draw_frame.step),
                        QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);

            ui->speedLabel->setText(QString("Speed: ") + QString::number(car_status->getCarSpeed()) + QString(" km/h"));
        }

        qApp->processEvents();
    }
}

// Get the number of camera available
void MainWindow::refreshCams() {
    std::vector<struct Camera> cams;
    std::string path = "/sys/class/video4linux";
    for (const auto &entry : fs::directory_iterator(path)) {
        std::string str_path = fs::canonical(entry).u8string();

        // Get camera
        int v4l_id;
        std::regex v4l_id_ex("video[\\d]+$");
        std::smatch v4l_id_sm;
        if (regex_search(str_path, v4l_id_sm, v4l_id_ex)) {
            std::cout << v4l_id_sm.str() << std::endl;
            std::string tmp = v4l_id_sm.str();
            v4l_id = std::stoi(tmp.substr(5, tmp.size() - 5));
        } else {
            continue;
        }

        // Get camera identifier
        std::string identifier;
        std::regex identifier_ex("\\/[\\d]+\\-[\\d]+\\.[\\d]+/");
        std::smatch identifier_sm;
        if (regex_search(str_path, identifier_sm, identifier_ex)) {
            identifier = identifier_sm.str();
        } else {
            identifier = std::to_string(v4l_id);
        }

        cams.push_back(Camera(v4l_id, identifier));
    }

}

void MainWindow::setInputSource(InputSource input_source) {
    this->input_source = input_source;
}

void MainWindow::setSimulation(Simulation *simulation) {
    this->simulation = simulation;
}

void MainWindow::setSpeedLimit(MaxSpeedLimit speed_limit) {
    std::lock_guard<std::mutex> guard(speed_limit_mutex);
    this->speed_limit = speed_limit;
}

MaxSpeedLimit MainWindow::getSpeedLimit() {
    std::lock_guard<std::mutex> guard(speed_limit_mutex);
    return speed_limit;
}

void MainWindow::openSimulationSelector() {
    this->simulation->showFullScreen();
}

void MainWindow::toggleMute() {

    if (is_mute) {
        is_mute = false;
        this->ui->muteBtn->setIcon(QIcon(":/resources/images/volume.png"));
    } else {
        is_mute = true;
        this->ui->muteBtn->setIcon(QIcon(":/resources/images/mute.png"));
    }

}

void MainWindow::toggleAlert() {

    if (is_alert) {
        is_alert = false;
        this->ui->alertBtn->setText(QString("Alert: Off"));
    } else {
        is_alert = true;
        this->ui->alertBtn->setText(QString("Alert: On"));
    }

}

void MainWindow::showCameraWizard() {
    camera_model->showCameraWizard();
}