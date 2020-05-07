#include "main_window.h"
#include "ui_main_window.h"
#include "config.h"

using namespace cv;
using namespace ml_cam;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Connect buttons
    connect(ui->menuBtn, SIGNAL(released()), this, SLOT(changeCamClicked()));
    connect(ui->alertBtn, SIGNAL(released()), this, SLOT(changeCamClicked()));

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

    object_detector_with_tracking =
        std::make_shared<ObjectDetectorWithTracking>();
    lane_detector = std::make_shared<LaneDetector>();
    car_prop_reader = std::make_shared<CarPropReader>();

    // Start processing threads
    std::thread od_thread(&MainWindow::objectTrackingThread,
                          object_detector_with_tracking, std::ref(current_img),
                          std::ref(current_img_mutex),
                          std::ref(object_tracking_results),
                          std::ref(object_tracking_results_mutex));
    od_thread.detach();

#ifndef DISABLE_LANE_DETECTOR
    std::thread ld_thread(&MainWindow::laneDetectionThread, lane_detector,
                          this);
    ld_thread.detach();
#endif

    std::thread cpr_thread(&MainWindow::carPropReaderThread,
                           car_prop_reader);
    cpr_thread.detach();
}

void MainWindow::objectTrackingThread(
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

void MainWindow::laneDetectionThread(
    std::shared_ptr<LaneDetector> lane_detector, MainWindow *main_ptr) {
    cv::Mat clone_img;
    while (true) {
        {
            std::lock_guard<std::mutex> guard(main_ptr->current_img_mutex);
            clone_img = main_ptr->current_img.clone();
        }

        if (clone_img.empty()) {
            continue;
        }

        #if defined (DEBUG_LANE_DETECTOR_SHOW_LINES)  || defined (DEBUG_LANE_DETECTOR_SHOW_LINE_MASK)
        cv::Mat lane_line_mask_copy;
        cv::Mat detected_line_img_copy;
        cv::Mat reduced_line_img_copy;
        std::vector<LaneLine> results = lane_detector->detectLaneLines(clone_img, lane_line_mask_copy, detected_line_img_copy, reduced_line_img_copy);
        {
            std::lock_guard<std::mutex> guard(main_ptr->lane_detection_results_mutex);
            main_ptr->lane_line_mask = lane_line_mask_copy;
            main_ptr->detected_line_img = detected_line_img_copy;
            main_ptr->reduced_line_img = reduced_line_img_copy;
            main_ptr->lane_detection_results = results;
        }
        #else
        std::vector<LaneLine> results = lane_detector->detectLaneLines(clone_img);
        {
            std::lock_guard<std::mutex> guard(main_ptr->lane_detection_results_mutex);
            main_ptr->lane_detection_results = results;
        }
        #endif 
    }
}

void MainWindow::carPropReaderThread(
    std::shared_ptr<CarPropReader> car_prop_reader) {
    while (true) {
        car_prop_reader->updateProps();
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::changeCamClicked() {
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

void MainWindow::startVideoGrabber() {

    Mat frame;
    Mat draw_frame;

    while (true) {
        frame = simulation->getCurrentImage();

        if (!frame.empty()) {
            
            // Resize frame
            frame =  resizeByMaxSize(frame, IMG_MAX_SIZE);

            // Processing
            setCurrentImage(frame);

            draw_frame = getCurrentImage();

            #ifdef DEBUG_LANE_DETECTOR_SHOW_LINE_MASK
            cv::Mat lane_line_mask_copy;
            #endif

            #ifdef DEBUG_LANE_DETECTOR_SHOW_LINES
            cv::Mat detected_line_img_copy;
            cv::Mat reduced_line_img_copy;
            #endif

            std::vector<LaneLine> lane_detection_results_copy;
            
            {
                std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
                lane_detection_results_copy = lane_detection_results;

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINE_MASK
                lane_line_mask_copy = lane_line_mask.clone();
                #endif
                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINES
                detected_line_img_copy = detected_line_img.clone();
                reduced_line_img_copy = reduced_line_img.clone();
                #endif
            }
                
            if (!lane_detection_results.empty()) {

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

//             // Add speed
//             int gps_signal_status = car_prop_reader->getSignalStatus();
//             if (gps_signal_status != 0) {
//                 std::stringstream ss;
//                 ss << "No GPS";

// #ifdef SMARTCAM_DEBUG
//                 ss << " (" << gps_signal_status << ")";
// #endif

//                 putText(draw_frame, ss.str(),
//                         Point2f(20 - 2, draw_frame.rows - 20 - 2),
//                         FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 0), 3);
//                 putText(draw_frame, ss.str(), Point2f(20, draw_frame.rows - 20),
//                         FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255, 255), 3);
//             } else {
//                 std::stringstream ss;
//                 ss << car_prop_reader->getCarSpeed() << " km/h";
//                 putText(draw_frame, ss.str(),
//                         Point2f(20 - 2, draw_frame.rows - 20 - 2),
//                         FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 0), 3);
//                 putText(draw_frame, ss.str(), Point2f(20, draw_frame.rows - 20),
//                         FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255, 255), 3);
//             }

            // ### Show current image
            QImage qimg(draw_frame.data, static_cast<int>(draw_frame.cols),
                        static_cast<int>(draw_frame.rows),
                        static_cast<int>(draw_frame.step),
                        QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
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

void MainWindow::setInputSource(InputSource input_source) {
    this->input_source = input_source;
}

void MainWindow::setSimulation(Simulation *simulation) {
    this->simulation = simulation;
}