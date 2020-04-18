#if !defined(FILE_STORAGE_H)
#define FILE_STORAGE_H

#include <iostream>
#include "filesystem_include.h"
#include "utility.h"
#include <ctime>
#include <sstream>
#include <chrono>
#include <cstdlib>

namespace ml_cam {

class FileStorage {
   private:
    const fs::path DATA_FOLDER_NAME = "CarSmartCam";
    const fs::path PHOTO_FOLDER_NAME = "Photos";
    const fs::path VIDEO_FOLDER_NAME = "Videos";
    fs::path DATA_FOLDER;
    fs::path PHOTO_FOLDER;
    fs::path VIDEO_FOLDER;

    fs::path last_saved_item;

   public:
    FileStorage();
    ~FileStorage();

    fs::path getDataPath();
    fs::path getPhotoPath();
    fs::path getVideoPath();
    void initStorage();

    fs::path getLastSavedItem();
    void setLastSavedItem(fs::path);

    bool saveImage(const cv::Mat & img);
};

}  // namespace ml_cam

#endif // FILE_STORAGE_H
