#include "four_point_select_page.h"

using namespace std;
using namespace cv;

FourPointSelectPage::FourPointSelectPage(QWidget *parent,
                                         std::shared_ptr<CarStatus> car_status)
    : QWizardPage(parent), ui(new Ui::FourPointSelectPage) {
    ui->setupUi(this);
    this->car_status = car_status;
    getNewImageFromCam();

    QDoubleSpinBox *tl_x_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *tl_y_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *tr_x_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *tr_y_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *br_x_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *br_y_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *bl_x_box = getDummyDoubleSpinBox();
    QDoubleSpinBox *bl_y_box = getDummyDoubleSpinBox();
    registerField("tl_x", tl_x_box, "value");
    registerField("tl_y", tl_y_box, "value");
    registerField("tr_x", tr_x_box, "value");
    registerField("tr_y", tr_y_box, "value");
    registerField("br_x", br_x_box, "value");
    registerField("br_y", br_y_box, "value");
    registerField("bl_x", bl_x_box, "value");
    registerField("bl_y", bl_y_box, "value");

    {
        std::lock_guard<std::mutex> guard(points_mutex);

        float tl_x = 0.3, tl_y = 0.3;
        tl_x_box->setValue(tl_x);
        tl_y_box->setValue(tl_y);
        points.push_back(Point2f(tl_x, tl_y));

        float tr_x = 0.7, tr_y = 0.3;
        tr_x_box->setValue(tr_x);
        tr_y_box->setValue(tr_y);
        points.push_back(Point2f(tr_x, tr_y));

        float br_x = 0.7, br_y = 0.7;
        br_x_box->setValue(br_x);
        br_y_box->setValue(br_y);
        points.push_back(Point2f(br_x, br_y));

        float bl_x = 0.3, bl_y = 0.7;
        bl_x_box->setValue(bl_x);
        bl_y_box->setValue(bl_y);
        points.push_back(Point2f(bl_x, bl_y));

        current_point_id = 0;
        current_point_title = "Top-Left";
    }

    pointToScrollbar();
    connect(ui->getImageBtn, SIGNAL(released()), this,
            SLOT(getNewImageFromCam()));
    connect(ui->verticalScrollBar, SIGNAL(sliderMoved(int)), this,
            SLOT(updateY(int)));
    connect(ui->horizontalScrollBar, SIGNAL(sliderMoved(int)), this,
            SLOT(updateX(int)));
    connect(ui->topLeftBtn, SIGNAL(released()), this,
            SLOT(selectTopLeftPoint()));
    connect(ui->topRightBtn, SIGNAL(released()), this,
            SLOT(selectTopRightPoint()));
    connect(ui->bottomRightBtn, SIGNAL(released()), this,
            SLOT(selectBottomRightPoint()));
    connect(ui->bottomLeftBtn, SIGNAL(released()), this,
            SLOT(selectBottomLeftPoint()));

    getNewImageFromCam();
    updateVisualization();
}

QDoubleSpinBox *FourPointSelectPage::getDummyDoubleSpinBox() {
    QDoubleSpinBox *dummy = new QDoubleSpinBox(this);
    dummy->setVisible(false);
    return dummy;
}

void FourPointSelectPage::selectTopLeftPoint() { selectPoint("Top-Left", 0); }

void FourPointSelectPage::selectTopRightPoint() { selectPoint("Top-Right", 1); }

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
    ui->verticalScrollBar->setValue(points[current_point_id].y *
                                    ui->verticalScrollBar->maximum());
    ui->horizontalScrollBar->setValue(points[current_point_id].x *
                                      ui->horizontalScrollBar->maximum());
}

void FourPointSelectPage::updateY(int value) {
    float fval = static_cast<float>(value) / ui->verticalScrollBar->maximum();
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        points[current_point_id].y = fval;
    }
    setField(QString::fromStdString(prefix[current_point_id] + "y"), fval);
    updateVisualization();
}

void FourPointSelectPage::updateX(int value) {
    float fval = static_cast<float>(value) / ui->horizontalScrollBar->maximum();
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        points[current_point_id].x = fval;
    }
    setField(QString::fromStdString(prefix[current_point_id] + "x"), fval);
    updateVisualization();
}

void FourPointSelectPage::getNewImageFromCam() {
    {
        std::lock_guard<std::mutex> guard(image_mutex);
        car_status->getCurrentImage(image);
    }
    updateVisualization();
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

    int x;
    int y;
    std::string title;
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        x = points[current_point_id].x * viz_image.cols;
        y = points[current_point_id].y * viz_image.rows;
        title = current_point_title;
    }

    // Draw
    cv::putText(viz_image, title, cv::Point2f(10, 20), cv::FONT_HERSHEY_PLAIN,
                0.8, cv::Scalar(0, 0, 255, 255), 1);
    cv::line(viz_image, cv::Point(x, 0), cv::Point(x, viz_image.size().height),
             cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    cv::line(viz_image, cv::Point(0, y), cv::Point(viz_image.size().width, y),
             cv::Scalar(0, 255, 0), 1, cv::LINE_AA);


    std::vector<cv::Point> viz_points;
    {
        std::lock_guard<std::mutex> guard(points_mutex);
        for (size_t i = 0; i < points.size(); ++i) {
            int x = points[i].x * viz_image.cols;
            int y = points[i].y * viz_image.rows;
            viz_points.push_back(cv::Point(x, y));
            cv::circle(viz_image, cv::Point(x, y), 3, cv::Scalar(0, 0, 255),
                       -1);
            cv::putText(viz_image, std::to_string(i + 1),
                        cv::Point2f(x + 20, y), cv::FONT_HERSHEY_PLAIN, 0.8,
                        cv::Scalar(0, 0, 255, 255), 1);
        }
    }

    cv::Point draw_p1, draw_p2;
    getLinePointinImageBorder(viz_points[1], viz_points[2],
        draw_p1, draw_p2, viz_image.rows, viz_image.cols);
    cv::line(viz_image, draw_p1, draw_p2,
             cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    getLinePointinImageBorder(viz_points[0], viz_points[3],
        draw_p1, draw_p2, viz_image.rows, viz_image.cols);
    cv::line(viz_image, draw_p1, draw_p2,
             cv::Scalar(0, 0, 255), 1, cv::LINE_AA);

    QImage qimg(viz_image.data, static_cast<int>(viz_image.cols),
                static_cast<int>(viz_image.rows),
                static_cast<int>(viz_image.step), QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimg.rgbSwapped());
    this->ui->vizImageLabel->setPixmap(pixmap);
}

void FourPointSelectPage::getLinePointinImageBorder(const cv::Point &p1_in,
                                                    const cv::Point &p2_in,
                                                    cv::Point &p1_out,
                                                    cv::Point &p2_out, int rows,
                                                    int cols) {
    double m =
        (double)(p1_in.y - p2_in.y) /
        (double)(p1_in.x - p2_in.x + std::numeric_limits<double>::epsilon());
    double b = p1_in.y - (m * p1_in.x);

    std::vector<cv::Point> border_point;
    double x, y;
    // test for the line y = 0
    y = 0;
    x = (y - b) / m;
    if (x > 0 && x < cols) border_point.push_back(cv::Point(x, y));

    // test for the line y = img.rows
    y = rows;
    x = (y - b) / m;
    if (x > 0 && x < cols) border_point.push_back(cv::Point(x, y));

    // check intersection with horizontal lines x = 0
    x = 0;
    y = m * x + b;
    if (y > 0 && y < rows) border_point.push_back(cv::Point(x, y));

    x = cols;
    y = m * x + b;
    if (y > 0 && y < rows) border_point.push_back(cv::Point(x, y));

    p1_out = border_point[0];
    p2_out = border_point[1];
}