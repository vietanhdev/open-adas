#include "four_point_select_page.h"

using namespace std;

FourPointSelectPage::FourPointSelectPage(QWidget *parent, CarStatus *car_status) :
    QWizardPage(parent),
    ui(new Ui::FourPointSelectPage)
{
    ui->setupUi(this);
    this->car_status = car_status;
    getNewImageFromCam();

    {
        std::lock_guard<std::mutex> guard(points_mutex);
        points.push_back(Point(0.3, 0.3));
        points.push_back(Point(0.7, 0.3));
        points.push_back(Point(0.7, 0.7));
        points.push_back(Point(0.3, 0.7));
        current_point_id = 0;
        current_point_title = "Top-Left";
    }

    pointToScrollbar();
    connect(ui->getImageBtn, SIGNAL(released()), this, SLOT(getNewImageFromCam()));
    connect(ui->verticalScrollBar, SIGNAL(sliderMoved(int)), this, SLOT(updateY(int)));
    connect(ui->horizontalScrollBar, SIGNAL(sliderMoved(int)), this, SLOT(updateX(int)));
    connect(ui->topLeftBtn, SIGNAL(released()), this, SLOT(selectTopLeftPoint()));
    connect(ui->topRightBtn, SIGNAL(released()), this, SLOT(selectTopRightPoint()));
    connect(ui->bottomRightBtn, SIGNAL(released()), this, SLOT(selectBottomRightPoint()));
    connect(ui->bottomLeftBtn, SIGNAL(released()), this, SLOT(selectBottomLeftPoint()));

    // Fields
    // registerField("carWidthInput", ui->carWidthInput);
    // registerField("carpetWidthInput", ui->carpetWidthInput);
    // registerField("carToCarpetDistanceInput", ui->carToCarpetDistanceInput);
    // registerField("carpetLengthInput", ui->carpetLengthInput);

    getNewImageFromCam();
    updateVisualization();
}

void FourPointSelectPage::selectTopLeftPoint() {
    selectPoint("Top-Left", 0);
}
void FourPointSelectPage::selectTopRightPoint() {
    selectPoint("Top-Right", 1);
}
void FourPointSelectPage::selectBottomRightPoint() {
    selectPoint("Bottom-Right", 2);
}
void FourPointSelectPage::selectBottomLeftPoint() {
    selectPoint("Bottom-Left", 3);
}

void FourPointSelectPage::selectPoint(std::string title, int point_id) {
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        current_point_id = point_id;
        current_point_title = title;
    }
    pointToScrollbar();
    updateVisualization();
}

void FourPointSelectPage::pointToScrollbar() {
    ui->verticalScrollBar->setValue(points[current_point_id].y * ui->verticalScrollBar->maximum());
    ui->horizontalScrollBar->setValue(points[current_point_id].x * ui->horizontalScrollBar->maximum());
}

void FourPointSelectPage::updateY(int value) {
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        points[current_point_id].y = static_cast<float>(value) / ui->verticalScrollBar->maximum();
    }
    updateVisualization();
}

void FourPointSelectPage::updateX(int value) {
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        points[current_point_id].x = static_cast<float>(value) / ui->horizontalScrollBar->maximum();
    }
    updateVisualization();
}

void FourPointSelectPage::getNewImageFromCam() {
   setImage(car_status->getCurrentImage());
   updateVisualization();
}

void FourPointSelectPage::setImage(const cv::Mat &image) {
    std::lock_guard<std::mutex> guard(image_mutex);
    this->image = image;
}

cv::Mat FourPointSelectPage::getImage() {
    std::lock_guard<std::mutex> guard(image_mutex);
    return image;
}

void FourPointSelectPage::updateVisualization() {

    cv::Mat viz_image;
    {
        std::lock_guard<std::mutex> guard(image_mutex);
        viz_image = image.clone();
    }

    if (viz_image.empty()) return;

    int x; int y; std::string title;
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        x = points[current_point_id].x * viz_image.cols;
        y = points[current_point_id].y * viz_image.rows;
        title = current_point_title;
    }

    // Draw
    cv::putText(viz_image, title, cv::Point2f(10,20), cv::FONT_HERSHEY_PLAIN, 0.8,  cv::Scalar(0,0,255,255), 1);
    cv::line(viz_image, cv::Point(x, 0), cv::Point(x, viz_image.size().height), cv::Scalar(0,255,0), 1, cv::LINE_AA);
    cv::line(viz_image, cv::Point(0, y), cv::Point(viz_image.size().width, y), cv::Scalar(0,255,0), 1, cv::LINE_AA);

    {
        std::lock_guard<std::mutex> guard(points_mutex);
        for (size_t i = 0; i < points.size(); ++i) {
            int x = points[i].x * viz_image.cols;
            int y = points[i].y * viz_image.rows;
            cv::circle(viz_image, cv::Point(x, y), 3, cv::Scalar(0,0,255), -1);
            cv::putText(viz_image, std::to_string(i+1), cv::Point2f(x+20, y), cv::FONT_HERSHEY_PLAIN, 0.8,  cv::Scalar(0,0,255,255), 1);
        }
        
    } 
    
    QImage qimg(viz_image.data, static_cast<int>(viz_image.cols),
                static_cast<int>(viz_image.rows),
                static_cast<int>(viz_image.step),
                QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimg.rgbSwapped());
    this->ui->vizImageLabel->setPixmap(pixmap);

}