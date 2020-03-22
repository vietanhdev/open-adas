#include <QApplication>
#include <stdlib.h>
#include <iostream>
#include "filesystem_include.h"
#include "dark_style.h"
#include "main_window.h"
#include "file_storage.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Init file storage
    ml_cam::FileStorage fs;
    fs.initStorage();

    // Style our application with custom dark style
    a.setStyle(new DarkStyle);
    
    // Create our mainwindow instance
    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    mainWindow->showCam();

    return a.exec();
}
