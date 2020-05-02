#include "main_window.h"

#include "config.h"
#include "ui_main_window.h"

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Connect buttons
    connect(ui->changeCamBtn, SIGNAL(released()), this,
            SLOT(changeCam_clicked()));
    connect(ui->menuBtn, SIGNAL(released()), this, SLOT(changeCam_clicked()));
    connect(ui->alertBtn, SIGNAL(released()), this, SLOT(changeCam_clicked()));

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

    object_detector_with_tracking =
        std::make_shared<ObjectDetectorWithTracking>();
    lane_detector = std::make_shared<LaneDetector>();
    car_prop_reader = std::make_shared<CarPropReader>();

    // Start processing threads
    std::thread od_thread(&MainWindow::object_tracking_thread,
                          object_detector_with_tracking, std::ref(current_img),
                          std::ref(current_img_mutex),
                          std::ref(object_tracking_results),
                          std::ref(object_tracking_results_mutex));
    od_thread.detach();

#ifndef DISABLE_LANE_DETECTOR
    std::thread ld_thread(&MainWindow::lane_detection_thread, lane_detector,
                          std::ref(current_img), std::ref(current_img_mutex),
                          std::ref(lane_detection_results),
                          std::ref(lane_detection_results_mutex));
    ld_thread.detach();
#endif

    std::thread cpr_thread(&MainWindow::car_prop_reader_thread,
                           car_prop_reader);
    cpr_thread.detach();
}

void MainWindow::setInputVideo(std::string video_path) {
    this->input_from_video = true;
    this->video_path = video_path;
}

void MainWindow::object_tracking_thread(
    std::shared_ptr<ObjectDetectorWithTracking> object_detector_with_tracking,
    cv::Mat &img, std::mutex &img_mutex,
    std::vector<TrackingObject> &object_tracking_results,
    std::mutex &object_tracking_results_mutex) {
    cv::Mat clone_img;
    while (true) {
        {
            std::lock_guard<std::mutex> guard(img_mutex);
            clone_img = img.clone();
        }

        if (clone_img.empty()) {
            continue;
        }

        // std::vector<Detection> results;
        // results = object_detector->inference(clone_img);

        std::vector<TrackingObject> tracks;
        if (0 != object_detector_with_tracking->runDetectAndTrack(clone_img,
                                                              tracks)) {
            cerr << "Error on detecting and tracking!" << endl;
        }

        {
            std::lock_guard<std::mutex> guard(object_tracking_results_mutex);
            object_tracking_results = tracks;
        }
    }
}

void MainWindow::lane_detection_thread(
    std::shared_ptr<LaneDetector> lane_detector, cv::Mat &img,
    std::mutex &img_mutex, cv::Mat &lane_detection_results,
    std::mutex &lane_detection_results_mutex) {
    cv::Mat clone_img;
    while (true) {
        {
            std::lock_guard<std::mutex> guard(img_mutex);
            clone_img = img.clone();
        }

        if (clone_img.empty()) {
            continue;
        }

        cv::Mat results = lane_detector->detectLane(clone_img);
        {
            std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
            lane_detection_results = results;
        }
    }
}

void MainWindow::car_prop_reader_thread(
    std::shared_ptr<CarPropReader> car_prop_reader) {
    while (true) {
        car_prop_reader->updateProps();
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::changeCam_clicked() {
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    SDL_LoadWAV("sounds/shutter-fast.wav", &wavSpec, &wavBuffer, &wavLength);

    // open audio device
    SDL_AudioDeviceID deviceId =
        SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    // play audio
    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);

    // keep window open enough to hear the sound
    SDL_Delay(200);

    // clean up
    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (video.isOpened()) {
        video.release();
    }
    QApplication::quit();
    exit(0);
}

void MainWindow::showCam(std::string video_path) {
    setInputVideo(video_path);
    showCam();
}

std::string MainWindow::getInputVideoPath() { return video_path; }

void MainWindow::showCam() {
    if (input_from_video) {
        video.open(getInputVideoPath());
    } else if (!video.open(current_camera_index)) {
        QMessageBox::critical(
            this, "Camera Error",
            "Make sure you entered a correct camera index,"
            "<br>or that the camera is not being accessed by another program!");
        return;
    }

    Mat frame;
    Mat draw_frame;
    while (true) {
        // If we still cannot open camera, exit the program
        if (!video.isOpened()) {
            QMessageBox::critical(
                this, "Camera Error",
                "Make sure you entered a correct camera index,"
                "<br>or that the camera is not being accessed by another "
                "program!");
            exit(1);
        }

        video >> frame;

        if (!frame.empty()) {
            // Processing
            setCurrentImage(frame);

            draw_frame = getCurrentImage();
            {
                std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
                if (!lane_detection_results.empty()) {
                    cv::Mat lane_result = lane_detection_results.clone();
                    cv::resize(lane_result, lane_result, draw_frame.size());

                    cv::Mat rgb_lane_result =
                        cv::Mat::zeros(draw_frame.size(), CV_8UC3);

                    rgb_lane_result.setTo(Scalar(0, 0, 255), lane_result > 0.5);
                    draw_frame.setTo(Scalar(0, 0, 0), lane_result > 0.5);

                    cv::addWeighted(draw_frame, 1, rgb_lane_result, 1, 0,
                                    draw_frame);
                }
            }

            // {
            //     std::lock_guard<std::mutex>
            //     guard(object_detection_results_mutex); if
            //     (!object_detection_results.empty()) {
            //         draw_object_detection_results(object_detection_results,
            //         draw_frame, cv::Scalar(0,255,0), false);
            //     }
            // }
            {
                std::lock_guard<std::mutex> guard(
                    object_tracking_results_mutex);
                if (!object_tracking_results.empty()) {
                    for (int i = 0; i < object_tracking_results.size(); ++i) {
                        object_detector_with_tracking->drawTrack(
                            draw_frame, 1.0, object_tracking_results[i], true);
                    }
                }
            }

            // Add speed
            int gps_signal_status = car_prop_reader->getSignalStatus();
            if (gps_signal_status != 0) {
                std::stringstream ss;
                ss << "No GPS";

#ifdef SMARTCAM_DEBUG
                ss << " (" << gps_signal_status << ")";
#endif

                putText(draw_frame, ss.str(),
                        Point2f(80 - 2, draw_frame.rows - 80 - 2),
                        FONT_HERSHEY_PLAIN, 5, Scalar(0, 0, 0), 5);
                putText(draw_frame, ss.str(), Point2f(80, draw_frame.rows - 80),
                        FONT_HERSHEY_PLAIN, 5, Scalar(0, 0, 255, 255), 5);
            } else {
                std::stringstream ss;
                ss << car_prop_reader->getCarSpeed() << " km/h";
                putText(draw_frame, ss.str(),
                        Point2f(80 - 2, draw_frame.rows - 80 - 2),
                        FONT_HERSHEY_PLAIN, 5, Scalar(0, 0, 0), 5);
                putText(draw_frame, ss.str(), Point2f(80, draw_frame.rows - 80),
                        FONT_HERSHEY_PLAIN, 5, Scalar(0, 0, 255, 255), 5);
            }

            // ### Show current image
            QImage qimg(draw_frame.data, static_cast<int>(draw_frame.cols),
                        static_cast<int>(draw_frame.rows),
                        static_cast<int>(draw_frame.step),
                        QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        } else if (input_from_video) {
            video.release();
            video.open(getInputVideoPath());
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

    available_cams = cams;
}

void MainWindow::setCurrentImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    current_img = img.clone();
}

cv::Mat MainWindow::getCurrentImage() {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.clone();
}