#include "main_window.h"
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
    connect(ui->alertBtn, SIGNAL(released()), this,
            SLOT(changeCam_clicked()));

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

    object_detector = new ObjectDetector();
    lane_detector = new LaneDetector();

    // Start processing threads
    std::thread od_thread(&MainWindow::object_detection_thread, object_detector, std::ref(current_img), std::ref(current_img_mutex),
                    std::ref(object_detection_results), std::ref(object_detection_results_mutex)
                );
    std::thread ld_thread(&MainWindow::lane_detection_thread, lane_detector, std::ref(current_img), std::ref(current_img_mutex),
                    std::ref(lane_detection_results), std::ref(lane_detection_results_mutex)
                );
    od_thread.detach();
    ld_thread.detach(); 
}


void MainWindow::setInputVideo(std::string video_path) {
    this->input_from_video = true;
    this->video_path = video_path;
}


void MainWindow::object_detection_thread(ObjectDetector * object_detector, cv::Mat & img, std::mutex & img_mutex, 
    std::vector<Detection> & object_detection_results, std::mutex & object_detection_results_mutex) {
    
    cv::Mat clone_img;
    while (true) {
        {
            std::lock_guard<std::mutex> guard(img_mutex);
            clone_img = img.clone();
        }

        if (clone_img.empty()) {
            continue;
        }

        std::vector<Detection> results;
        results = object_detector->inference(clone_img);

        {
            std::lock_guard<std::mutex> guard(object_detection_results_mutex);
            object_detection_results = results;
        }
    }
    
}


void MainWindow::lane_detection_thread(LaneDetector * lane_detector, cv::Mat & img, std::mutex & img_mutex, 
    cv::Mat & lane_detection_results, std::mutex & lane_detection_results_mutex) {

    lane_detector->init();
    
    cv::Mat clone_img;
    while (true) {
        {
            std::lock_guard<std::mutex> guard(img_mutex);
            clone_img = img.clone();
        }

        if (clone_img.empty()) {
            continue; 
        }

        cv::Mat results = lane_detector->detect_lane(clone_img);
        {
            std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
            lane_detection_results = results;
        }
    }
    
}


MainWindow::~MainWindow() { delete ui; }

void MainWindow::changeCam_clicked() {
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    SDL_LoadWAV("sounds/shutter-fast.wav", &wavSpec, &wavBuffer, &wavLength);
	
    // open audio device
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

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

std::string MainWindow::getInputVideoPath() {
    return video_path;
}

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
    while (true) {

        // User changed camera
        if (selected_camera_index != current_camera_index) {
            
            video.release();
            refreshCams();
            current_camera_index = selected_camera_index;
            video.open(current_camera_index);

        } else if (!video.isOpened()) {

            // Reset to default camera (0)
            refreshCams();
            current_camera_index = selected_camera_index;

        }

        // If we still cannot open camera, exit the program
        if (!video.isOpened()) {
            QMessageBox::critical(
            this, "Camera Error",
            "Make sure you entered a correct camera index,"
            "<br>or that the camera is not being accessed by another program!");
            exit(1);
        }

        video >> frame;

        if (!frame.empty()) {

            {
                std::lock_guard<std::mutex> guard(lane_detection_results_mutex);
                if (!lane_detection_results.empty()) {
                    cv::Mat lane_result = lane_detection_results.clone();
                    cv::resize(lane_result, lane_result, frame.size());
                    cv::addWeighted(frame, 1, lane_result, 0.5, 0, frame);
                }
            }

            {
                std::lock_guard<std::mutex> guard(object_detection_results_mutex);
                if (!object_detection_results.empty()) {
                    cv::RNG rng(244);
                    std::vector<cv::Scalar> color = { cv::Scalar(0,255,0), cv::Scalar(0,255,0) };
                    draw_object_detection_results(object_detection_results, frame, color, false);
                }
            }
            
            // Processing
            setCurrentImage(frame);

            // ### Show current image
            QImage qimg(frame.data, static_cast<int>(frame.cols),
                        static_cast<int>(frame.rows),
                        static_cast<int>(frame.step), QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        } else if (input_from_video) {
            video.release();
            video.open(getInputVideoPath());
        }

        qApp->processEvents();
    }
}


void MainWindow::refreshCams() {

    // Get the number of camera available
    cv::VideoCapture temp_camera;
    for (int i = 0; i < MAX_CAMS; ++i) {
        cv::VideoCapture temp_camera(i);
        bool fail = (!temp_camera.isOpened());
        temp_camera.release();
    }
}



void MainWindow::setCurrentImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    current_img = img.clone();
}

cv::Mat MainWindow::getCurrentImage() {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.clone();
}