#include "file_storage.h"

using namespace ml_cam;

FileStorage::FileStorage() {
    DATA_FOLDER = fs::path(getHomePath()) / DATA_FOLDER_NAME;
    PHOTO_FOLDER = DATA_FOLDER / PHOTO_FOLDER_NAME;
    VIDEO_FOLDER = DATA_FOLDER / VIDEO_FOLDER_NAME;
}

FileStorage::~FileStorage() {}

void FileStorage::initStorage() {
    std::cout << "Data Folder: " << getDataPath() << std::endl;
    std::cout << "Photos Folder: " << getPhotoPath() << std::endl;
    std::cout << "Videos Folder: " << getVideoPath() << std::endl;

    // *** Create directories

    // Photos
    if (fs::exists(getPhotoPath()) && !fs::is_directory(getPhotoPath())) {
        std::cerr << "Photos folder path is not a directory: " << getPhotoPath()
                  << std::endl;
        exit(-1);
    } else if (!fs::exists(getPhotoPath())) {
        // Create photos directory if not exist
        fs::create_directories(getPhotoPath());

        if (!fs::exists(getPhotoPath())) {
            std::cerr << "Could not create directory: " << getPhotoPath()
                      << std::endl;
            exit(-1);
        }
    }

    // Videos
    if (fs::exists(getVideoPath()) && !fs::is_directory(getVideoPath())) {
        std::cerr << "Videos folder path is not a directory: " << getVideoPath()
                  << std::endl;
        exit(-1);
    } else if (!fs::exists(getVideoPath())) {
        // Create photos directory if not exist
        fs::create_directories(getVideoPath());

        if (!fs::exists(getVideoPath())) {
            std::cerr << "Could not create directory: " << getVideoPath()
                      << std::endl;
            exit(-1);
        }
    }
}

fs::path FileStorage::getDataPath() { return DATA_FOLDER; }

fs::path FileStorage::getPhotoPath() { return PHOTO_FOLDER; }

fs::path FileStorage::getVideoPath() { return VIDEO_FOLDER; }

bool FileStorage::saveImage(const cv::Mat& img) {

    // *** Create a unique file name for each image
    // The image path is formated like this: .../CarSmartCam/Photos/2019-02-08-21-55-88.1549637628399.png

    std::stringstream filename;

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);

    filename << std::setw(4) << std::setfill('0') << local_tm.tm_year + 1900
    << "-" << std::setw(2) << std::setfill('0') << local_tm.tm_mon + 1 
    << "-" << std::setw(2) << std::setfill('0') << local_tm.tm_mday
    << "-" << std::setw(2) << std::setfill('0') << local_tm.tm_hour
    << "-" << std::setw(2) << std::setfill('0') << local_tm.tm_min 
    << "-" << std::setw(2) << std::setfill('0') << local_tm.tm_sec 
    << "." << millis << ".png";

    fs::path filepath = getPhotoPath() / fs::path(filename.str());
    setLastSavedItem(fs::path(filename.str()));

    // *** Save image to file
    cv::imwrite(filepath.string(), img);

    return true;
}


fs::path FileStorage::getLastSavedItem() {
    return last_saved_item;
}
void FileStorage::setLastSavedItem(fs::path path) {
    last_saved_item = path;
}