#include "lane_detector.h"
#include "config.h"

// Used line merging method from https://stackoverflow.com/questions/30746327/get-a-single-line-representation-for-multiple-close-by-lines-clustered-together/30904076

using namespace cv;

LaneDetector::LaneDetector() {
    UffModelParams params;
    params.inputW = INPUT_WIDTH;
    params.inputH = INPUT_HEIGHT;
    params.batchSize = 1;
    params.nClasses = 1;
    params.uffFilePath = SMARTCAM_LANE_DETECTION_MODEL;
    params.engineFilePath = SMARTCAM_LANE_DETECTION_TENSORRT_PLAN;
    params.forceRebuildEngine = false;
    params.inputTensorNames.push_back(INPUT_NODE);
    params.outputTensorNames.push_back(OUTPUT_NODE);
    params.fp16 = true;

    auto test = gLogger.defineTest("LaneDetector", 0, NULL);
    gLogger.reportTestStart(test);

    model = std::make_shared<Unet>(params);
}

cv::Mat LaneDetector::getLaneMask(const cv::Mat& input_img) {
    cv::Mat output_img;
    if (!model->infer(input_img, output_img)) {
        cerr << "Error on running lane detection model." << endl;
    }
    return output_img;
}

// Lane detect function
// For debug purpose
std::vector<LaneLine> LaneDetector::detectLaneLines(
    const cv::Mat& input_img, cv::Mat& line_mask, cv::Mat& detected_lines_img,
    cv::Mat& reduced_lines_img, bool &lane_departure) {

    // === Get binary lane mask ===
    line_mask = getLaneMask(input_img);

    // === Detect and reduce lines ===
    std::vector<cv::Vec4i> lines =
        detectAndReduceLines(line_mask, detected_lines_img, reduced_lines_img);

    std::vector<cv::Vec4i> filtered_lines;

    // Filter short lines
    int img_height = input_img.rows;
    for (size_t i = 0; i < lines.size(); ++i) {
        int line_length = sqrt(pow(lines[i][2] - lines[i][0], 2)
            + pow(lines[i][3] - lines[i][1], 2));
        if (line_length > 0.2 * img_height) {
            filtered_lines.push_back(lines[i]);
        }
    }
    lines = filtered_lines;

    // Filter all horizontal lines
    int n_filtered_horizontal_lines = 0;
    int n_horizontal_lines_20_degrees = 0;
    // std::vector<cv::Vec4i> filtered_lines;
    filtered_lines.clear();
    for (size_t i = 0; i < lines.size(); ++i) {
        double angle = atan2(lines[i][3] - lines[i][1], lines[i][2] - lines[i][0]) * 180.0 / CV_PI;
        if (angle < 0) angle = angle + 360;
        if (angle > 180) angle = 360 - angle;
        if (angle < 150 && angle > 30) {
            filtered_lines.push_back(lines[i]);
        } else {
            ++n_filtered_horizontal_lines;
        }
        if (angle > 160 || angle > 20) {
            ++n_horizontal_lines_20_degrees;
        }
    }
    lines = filtered_lines;

    

    // === Classify lines ===
    // Extend lines
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Point p1 = Point(lines[i][0], lines[i][1]);
        cv::Point p2 = Point(lines[i][2], lines[i][3]);
        getLinePointinImageBorder(p1, p2, p1, p2, reduced_lines_img.rows, reduced_lines_img.cols);
        lines[i] = cv::Vec4i(p1.x, p1.y, p2.x, p2.y);
    }
    std::vector<LaneLine> lane_lines;
    for (size_t i = 0; i < lines.size(); i++) {
        lane_lines.push_back(LaneLine(lines[i], OtherLaneLine));
    }
    cv::Mat line_segmentation = Mat::zeros(detected_lines_img.rows, detected_lines_img.cols, CV_8UC1);
    for (size_t i = 0; i < lane_lines.size(); ++i) {
        cv::Point p1 = Point(lane_lines[i].line[0], lane_lines[i].line[1]);
        cv::Point p2 = Point(lane_lines[i].line[2], lane_lines[i].line[3]);
        cv::line(line_segmentation, p1, p2, cv::Scalar(i+1), 1);
    }

    // Search for lines
    int center_point = line_segmentation.cols / 2;
    bool found_left = false; float left_pos = -1;
    bool found_right = false; float right_pos = 1;

    // For left line - Search on bottom edge
    int last_row = line_segmentation.rows - 1;
    for (int x = center_point; x >= 0; --x) {
        int line_id = static_cast<int>(line_segmentation.at<uchar>(cv::Point(x, last_row))) - 1;
        left_pos = x - center_point;
        if (line_id >= 0) {
            found_left = true;
            lane_lines[line_id].type = LeftLaneLine;
            break;
        }
    }

    // If not found, search on left edge
    if (!found_left) {
        for (int y = last_row; y >= 0.95*last_row; --y) {
            int line_id = static_cast<int>(line_segmentation.at<uchar>(cv::Point(0, y))) - 1;
            if (line_id >= 0) {
                found_left = true;
                left_pos = -center_point - (last_row - y);
                lane_lines[line_id].type = LeftLaneLine;
                break;
            }
        }
    }

    // For right line - Search on bottom edge
    for (int x = center_point + 1; x < line_segmentation.cols; ++x) {
        int line_id = static_cast<int>(line_segmentation.at<uchar>(cv::Point(x, last_row))) - 1;
        right_pos = x - center_point;
        if (line_id >= 0) {
            found_right = true;
            lane_lines[line_id].type = RightLaneLine;
            break;
        }
    }

    // If not found, search on right edge
    if (!found_right) {
        for (int y = last_row; y >= 0.95*last_row; --y) {
            int line_id = static_cast<int>(line_segmentation.at<uchar>(cv::Point(line_segmentation.cols-1, y))) - 1;
            if (line_id >= 0) {
                found_right = true;
                left_pos = center_point + (last_row - y);
                lane_lines[line_id].type = RightLaneLine;
                break;
            }
        }
    }

    // === Lane departure warning ===
    lane_departure = false;

    // Remove expired tracking
    int n_remove = 0;
    for (size_t i = 0; i < dual_line_checking_time.size(); ++i) {
        if (Timer::calcTimePassed(dual_line_checking_time[i]) > 3000) {
            ++n_remove;
        } else {
            break;
        }
    }
    if (n_remove > 0) {
        dual_line_checking_time.erase(dual_line_checking_time.begin() + n_remove - 1);
        is_dual_line.erase(is_dual_line.begin() + n_remove - 1);
    }
    dual_line_checking_time.push_back(Timer::getCurrentTime());
    is_dual_line.push_back(found_left && found_right);

    //  Calculate ratio of frames containing good line condition
    float good_frame_ratio = 0;
    if (is_dual_line.size() > 5) {
        int n_frames_good_lines = std::count(is_dual_line.begin(), is_dual_line.end(), true);
        good_frame_ratio = static_cast<float>(n_frames_good_lines) / is_dual_line.size();
    }
    
    if (good_frame_ratio > 0.6
        && found_left && found_right
        && n_filtered_horizontal_lines < 3
        && lane_lines.size() < 6) {

        // Normalize left pos, right pos to range(-xx.xx -> xx.xx)
        left_pos /= center_point;
        right_pos /= center_point;

        if ((abs(left_pos) < 0.3 && abs(right_pos) > 0.5)
        || (abs(left_pos) > 0.5 && abs(right_pos) < 0.3)) {
            lane_departure = true;
        }

    }

    // === Visualize ===
    reduced_lines_img = Mat::zeros(detected_lines_img.rows, detected_lines_img.cols, CV_8UC3);
    for (LaneLine line : lane_lines) {
        cv::Point p1 = Point(line.line[0], line.line[1]);
        cv::Point p2 = Point(line.line[2], line.line[3]);
        cv::Scalar color(50, 50, 50);
        if (line.type == LeftLaneLine) {
            color = cv::Scalar(255, 0, 0);
        } else if (line.type == RightLaneLine) {
            color = cv::Scalar(0, 0, 255);
        }
        cv::line(reduced_lines_img, p1, p2, color, 2);
    }
    if (lane_departure) {
        cv::putText(reduced_lines_img, "LANE DEPARTURE", Point2f(reduced_lines_img.cols / 2,reduced_lines_img.rows / 2), FONT_HERSHEY_PLAIN, 1.2,  Scalar(0,0,255,255), 1.5);
    }


    return lane_lines;
}

// Lane detect function
std::vector<LaneLine> LaneDetector::detectLaneLines(const cv::Mat& input_img, bool &lane_departure) {
    cv::Mat line_mask, detected_lines_img, reduced_lines_img;
    std::vector<LaneLine> lane_lines = detectLaneLines(
        input_img, line_mask, detected_lines_img, reduced_lines_img, lane_departure);

    return lane_lines;
}

std::vector<cv::Vec4i> LaneDetector::detectAndReduceLines(
    const cv::Mat& lane_prob, cv::Mat& detected_lines_img,
    cv::Mat& reduced_lines_img) {
    cv::Mat thresh = cv::Mat::zeros(lane_prob.size(), CV_8UC1);
    thresh.setTo(Scalar(255), lane_prob > 0.5);

    detected_lines_img = Mat::zeros(thresh.rows, thresh.cols, CV_8UC3);
    reduced_lines_img = detected_lines_img.clone();

    // Delect lines in any reasonable way
    std::vector<cv::Vec4i> lines;

    // Apply Hough Transform
    HoughLinesP(thresh, lines, 1, CV_PI / 180, 40, 5, 50);

    // partition via our partitioning function
    std::vector<int> labels;
    int equilavenceClassesCount = cv::partition(
        lines, labels, [](const cv::Vec4i l1, const cv::Vec4i l2) {
            return extendedBoundingRectangleLineEquivalence(
                l1, l2,
                // line extension length - as fraction of original line width
                0.1,
                // maximum allowed angle difference for lines to be considered
                // in same equivalence class
                2.0,
                // thickness of bounding rectangle around each line
                20);
        });

    // grab a random colour for each equivalence class
    RNG rng(215526);
    std::vector<Scalar> colors(equilavenceClassesCount);
    for (int i = 0; i < equilavenceClassesCount; i++) {
        colors[i] = Scalar(rng.uniform(30, 255), rng.uniform(30, 255),
                           rng.uniform(30, 255));
        ;
    }

    // draw original detected lines
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i& detectedLine = lines[i];
        line(detected_lines_img, cv::Point(detectedLine[0], detectedLine[1]),
             cv::Point(detectedLine[2], detectedLine[3]), colors[labels[i]], 2);
    }

    // build point clouds out of each equivalence classes
    std::vector<std::vector<Point2i>> pointClouds(equilavenceClassesCount);
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i& detectedLine = lines[i];
        pointClouds[labels[i]].push_back(
            Point2i(detectedLine[0], detectedLine[1]));
        pointClouds[labels[i]].push_back(
            Point2i(detectedLine[2], detectedLine[3]));
    }

    // fit line to each equivalence class point cloud
    std::vector<cv::Vec4i> reduced_lines = std::accumulate(
        pointClouds.begin(), pointClouds.end(), std::vector<cv::Vec4i>{},
        [](std::vector<cv::Vec4i> target,
           const std::vector<Point2i>& _pointCloud) {
            std::vector<Point2i> pointCloud = _pointCloud;

            // lineParams: [vx,vy, x0,y0]: (normalized vector, point on our
            // contour)
            // (x,y) = (x0,y0) + t*(vx,vy), t -> (-inf; inf)
            Vec4f lineParams;
            fitLine(pointCloud, lineParams, cv::DIST_L2, 0, 0.01, 0.01);

            // derive the bounding xs of point cloud
            decltype(pointCloud)::iterator minXP, maxXP;
            std::tie(minXP, maxXP) =
                std::minmax_element(pointCloud.begin(), pointCloud.end(),
                                    [](const Point2i& p1, const Point2i& p2) {
                                        return p1.x < p2.x;
                                    });

            // derive y coords of fitted line
            float m = lineParams[1] / lineParams[0];
            int y1 = ((minXP->x - lineParams[2]) * m) + lineParams[3];
            int y2 = ((maxXP->x - lineParams[2]) * m) + lineParams[3];

            target.push_back(cv::Vec4i(minXP->x, y1, maxXP->x, y2));
            return target;
        });
    lines = reduced_lines;

    return lines;
}

cv::Vec2d LaneDetector::linearParameters(cv::Vec4i line) {
    Mat a = (Mat_<double>(2, 2) << line[0], 1, line[2], 1);
    Mat y = (Mat_<double>(2, 1) << line[1], line[3]);
    cv::Vec2d mc;
    solve(a, y, mc);
    return mc;
}

cv::Vec4i LaneDetector::extendedLine(cv::Vec4i line, double d) {
    // oriented left-t-right
    Vec4d _line = line[2] - line[0] < 0
                      ? Vec4d(line[2], line[3], line[0], line[1])
                      : Vec4d(line[0], line[1], line[2], line[3]);
    double m = linearParameters(_line)[0];
    // solution of pythagorean theorem and m = yd/xd
    double xd = sqrt(d * d / (m * m + 1));
    double yd = xd * m;
    return Vec4d(_line[0] - xd, _line[1] - yd, _line[2] + xd, _line[3] + yd);
}

std::vector<Point2i> LaneDetector::boundingRectangleContour(cv::Vec4i line,
                                                            float d) {
    // finds coordinates of perpendicular lines with length d in both line
    // points https://math.stackexchange.com/a/2043065/183923

    cv::Vec2f mc = linearParameters(line);
    float m = mc[0];
    float factor = sqrtf((d * d) / (1 + (1 / (m * m))));

    float x3, y3, x4, y4, x5, y5, x6, y6;
    // special case(vertical perpendicular line) when -1/m -> -infinity
    if (m == 0) {
        x3 = line[0];
        y3 = line[1] + d;
        x4 = line[0];
        y4 = line[1] - d;
        x5 = line[2];
        y5 = line[3] + d;
        x6 = line[2];
        y6 = line[3] - d;
    } else {
        // slope of perpendicular lines
        float m_per = -1 / m;

        // y1 = m_per * x1 + c_per
        float c_per1 = line[1] - m_per * line[0];
        float c_per2 = line[3] - m_per * line[2];

        // coordinates of perpendicular lines
        x3 = line[0] + factor;
        y3 = m_per * x3 + c_per1;
        x4 = line[0] - factor;
        y4 = m_per * x4 + c_per1;
        x5 = line[2] + factor;
        y5 = m_per * x5 + c_per2;
        x6 = line[2] - factor;
        y6 = m_per * x6 + c_per2;
    }

    return std::vector<Point2i>{Point2i(x3, y3), Point2i(x4, y4),
                                Point2i(x6, y6), Point2i(x5, y5)};
}

bool LaneDetector::extendedBoundingRectangleLineEquivalence(
    const cv::Vec4i& _l1, const cv::Vec4i& _l2, float extensionLengthFraction,
    float maxAngleDiff, float boundingRectangleThickness) {
    cv::Vec4i l1(_l1), l2(_l2);
    // extend lines by percentage of line width
    float len1 = sqrtf((l1[2] - l1[0]) * (l1[2] - l1[0]) +
                       (l1[3] - l1[1]) * (l1[3] - l1[1]));
    float len2 = sqrtf((l2[2] - l2[0]) * (l2[2] - l2[0]) +
                       (l2[3] - l2[1]) * (l2[3] - l2[1]));
    cv::Vec4i el1 = extendedLine(l1, len1 * extensionLengthFraction);
    cv::Vec4i el2 = extendedLine(l2, len2 * extensionLengthFraction);

    // reject the lines that have wide difference in angles
    float a1 = atan(linearParameters(el1)[0]);
    float a2 = atan(linearParameters(el2)[0]);
    if (fabs(a1 - a2) > maxAngleDiff * M_PI / 180.0) {
        return false;
    }

    // calculate window around extended line
    // at least one point needs to inside extended bounding rectangle of other
    // line,
    std::vector<Point2i> lineBoundingContour =
        boundingRectangleContour(el1, boundingRectangleThickness / 2);
    return pointPolygonTest(lineBoundingContour, cv::Point(el2[0], el2[1]),
                            false) == 1 ||
           pointPolygonTest(lineBoundingContour, cv::Point(el2[2], el2[3]),
                            false) == 1;
}

void LaneDetector::getLinePointinImageBorder(const cv::Point &p1_in,
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
