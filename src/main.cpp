#include <stdlib.h>
#include <QApplication>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "dark_style.h"
#include "file_storage.h"
#include "filesystem_include.h"
#include "main_window.h"
#include "simulation.h"
#include "input_source.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    const std::string keys =
        "{help h usage ? |      | print this message   }"
        "{input_source   |simulation| input source. 'simulation' or 'camera'   }"
        "{input_video_path  |      | path to video file for simulation }"
        "{input_data_path   |      | path to data file for simulation  }"
        ;

    CommandLineParser parser(argc, argv, keys);
    parser.about("CarSmartCam v1.0.0");
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string input_source = parser.get<std::string>("input_source");
    std::string input_video_path = parser.get<std::string>("input_video_path");
    std::string input_data_path = parser.get<std::string>("input_data_path");

    // Style our application with custom dark style
    a.setStyle(new DarkStyle);

    // Create our mainwindow instance
    MainWindow *main_window = new MainWindow;
    main_window->show();
    // main_window->showFullScreen();

    // Prevent multithreading error on OpenCV
    // TODO: Fix this bug without setting n_threads to 1
    cv::setNumThreads(1);

    // Set input source and start camera getter
    if (input_source == "simulation") {

        // Create simulation window
        QWidget *simulation;

        if (input_video_path == "" && input_data_path == "") {
            simulation = new Simulation(&main_window->car_status);
        } else {
            simulation = new Simulation(&main_window->car_status, input_video_path, input_data_path);
        }
        
        simulation->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        simulation->show();

        // Set simulation
        main_window->setInputSource(InputFromSimulation);
        main_window->setSimulation((Simulation*)simulation);

    } else if (input_source == "camera") {
        
        cout << "Input from camera has not been supported yet." << endl;
        exit(1);

    }

    main_window->startVideoGrabber();

    return a.exec();
}
