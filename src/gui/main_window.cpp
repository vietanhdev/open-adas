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

    refreshCams();

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

    object_detector = new ObjectDetector();
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


void MainWindow::showCam() {

    if (!video.open(current_camera_index)) {
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
        frame = object_detector->inference(frame);
        if (!frame.empty()) {

            // Processing
            setCurrentImage(frame);

            // ### Show current image
            QImage qimg(frame.data, static_cast<int>(frame.cols),
                        static_cast<int>(frame.rows),
                        static_cast<int>(frame.step), QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
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