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
    connect(ui->menuBtn, SIGNAL(released()), this, SLOT(changeCamClicked()));
    connect(ui->alertBtn, SIGNAL(released()), this, SLOT(changeCamClicked()));

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

    object_detector = std::make_shared<ObjectDetector>();
    lane_detector = std::make_shared<LaneDetector>();
    car_gps_reader = std::make_shared<CarGPSReader>();

    // Start processing threads
    std::thread od_thread(&MainWindow::objectDetectionThread, 
        object_detector,
        &car_status);
    od_thread.detach();

#ifndef DISABLE_LANE_DETECTOR
    std::thread ld_thread(&MainWindow::laneDetectionThread, 
        lane_detector,
        &car_status);
    ld_thread.detach();
#endif

    std::thread cpr_thread(&MainWindow::carPropReaderThread,
                           car_gps_reader);
    cpr_thread.detach();
}

void MainWindow::objectDetectionThread(
    std::shared_ptr<ObjectDetector> object_detector,
    CarStatus *car_status) {
    cv::Mat clone_img;
    while (true) {
        clone_img = car_status->getCurrentImage();
        if (clone_img.empty()) {
            continue;
        }

        Timer::time_point_t begin_time = Timer::getCurrentTime();
        std::vector<TrafficObject> objects = object_detector->detect(clone_img);
        car_status->setObjectDetectionTime(Timer::calcTimePassed(begin_time));
        car_status->setDetectedObjects(objects);

    }
}

void MainWindow::laneDetectionThread(
    std::shared_ptr<LaneDetector> lane_detector, CarStatus *car_status) {
    cv::Mat clone_img;
    while (true) {
        clone_img = car_status->getCurrentImage();
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
    QApplication::quit();
    exit(0);
}

void MainWindow::startVideoGrabber() {

    Mat draw_frame;
    Timer::time_point_t last_fps_show = Timer::getCurrentTime();
    Timer::time_duration_t object_detection_time =
         car_status.getObjectDetectionTime();
    Timer::time_duration_t lane_detection_time =
         car_status.getLaneDetectionTime();
    while (true) {

        // Processing
        draw_frame = car_status.getCurrentImage();

        if (!draw_frame.empty()) {

            std::vector<LaneLine> detected_lane_lines = car_status.getDetectedLaneLines();
                
            if (!detected_lane_lines.empty()) {

                #ifdef DEBUG_LANE_DETECTOR_SHOW_LINE_MASK
                cv::Mat lane_line_mask_copy = car_status.getLineMask();
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
                        car_status.getObjectDetectionTime();
                    lane_detection_time =
                        car_status.getLaneDetectionTime();
                    last_fps_show = Timer::getCurrentTime();
                }

                cv::putText(draw_frame, "Object detection: " +  std::to_string(object_detection_time) + " ms", Point2f(10,20), FONT_HERSHEY_PLAIN, 1,  Scalar(0,0,255,255), 1.5);
                cv::putText(draw_frame, "Lane detection: " + std::to_string(lane_detection_time) + " ms", Point2f(10,40), FONT_HERSHEY_PLAIN, 1,  Scalar(0,0,255,255), 1.5);
                
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

            std::vector<TrafficObject> detected_objects = car_status.getDetectedObjects();

            if (!detected_objects.empty()) {
                object_detector->drawDetections(
                    detected_objects, draw_frame);
            }
    
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

}

void MainWindow::setInputSource(InputSource input_source) {
    this->input_source = input_source;
}

void MainWindow::setSimulation(Simulation *simulation) {
    this->simulation = simulation;
}