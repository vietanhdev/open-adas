#include "object_detector_with_tracking.h"

#include <ctime>
#include <future>
#include <iomanip>

#include "config.h"

///
/// \brief ObjectDetectorWithTracking::ObjectDetectorWithTracking
/// \param parser
///
ObjectDetectorWithTracking::ObjectDetectorWithTracking()
    : m_showLogs(true),
      m_fps(25),
      m_captureTimeOut(60000),
      m_trackingTimeOut(60000),
      m_isTrackerInitialized(false),
      m_isDetectorInitialized(false) {
    m_colors.push_back(cv::Scalar(255, 0, 0));
    m_colors.push_back(cv::Scalar(0, 255, 0));
    m_colors.push_back(cv::Scalar(0, 0, 255));
    m_colors.push_back(cv::Scalar(255, 255, 0));
    m_colors.push_back(cv::Scalar(0, 255, 255));
    m_colors.push_back(cv::Scalar(255, 0, 255));
    m_colors.push_back(cv::Scalar(255, 127, 255));
    m_colors.push_back(cv::Scalar(127, 0, 255));
    m_colors.push_back(cv::Scalar(127, 0, 127));
}

int ObjectDetectorWithTracking::runDetectAndTrack(const cv::Mat& frame, std::vector<TrackingObject> &tracks) {
    if (!m_isDetectorInitialized || !m_isTrackerInitialized) {
        cv::UMat ufirst = frame.getUMat(cv::ACCESS_READ);
        if (!m_isDetectorInitialized) {
            m_isDetectorInitialized = initDetector(ufirst);
            if (!m_isDetectorInitialized) {
                std::cerr << "CaptureAndDetect: Detector initialize error!!!"
                          << std::endl;
                return 1;
            }
        }
        if (!m_isTrackerInitialized) {
            m_isTrackerInitialized = initTracker(ufirst);
            if (!m_isTrackerInitialized) {
                std::cerr << "CaptureAndDetect: Tracker initialize error!!!"
                          << std::endl;
                return 1;
            }
        }
    }

    regions_t regions;
    detection(frame, regions);
    tracking(frame, regions);

    tracks = m_tracker->GetTracks();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return 0;
}

///
/// \brief ObjectDetectorWithTracking::Detection
/// \param frame
/// \param regions
///
void ObjectDetectorWithTracking::detection(const cv::Mat &frame, regions_t& regions) {

    // Inference
    std::vector<Detection> results;
    results = object_detector->inference(frame);

    // Convert detection result to regions
    regions.clear();
    regions = convertDetectionsToRegions(results);
}

///
/// \brief ObjectDetectorWithTracking::Tracking
/// \param frame
/// \param regions
///
void ObjectDetectorWithTracking::tracking(const cv::Mat &frame,
                                          const regions_t& regions) {
    cv::UMat uframe;
    if (m_tracker->CanColorFrameToTrack()) {
        uframe = frame.getUMat(cv::ACCESS_READ);
    } else {
        cv::cvtColor(frame, uframe, cv::COLOR_BGR2GRAY);
    }

    m_tracker->Update(regions, uframe, m_fps);
}

///
/// \brief ObjectDetectorWithTracking::DrawTrack
/// \param frame
/// \param resizeCoeff
/// \param track
/// \param drawTrajectory
///
void ObjectDetectorWithTracking::drawTrack(cv::Mat frame, int resizeCoeff,
                                           const TrackingObject& track,
                                           bool drawTrajectory) {
    auto ResizePoint = [resizeCoeff](const cv::Point& pt) -> cv::Point {
        return cv::Point(resizeCoeff * pt.x, resizeCoeff * pt.y);
    };

    cv::Scalar color =
        track.m_isStatic ? cv::Scalar(255, 0, 255) : cv::Scalar(0, 255, 0);
    cv::Point2f rectPoints[4];
    track.m_rrect.points(rectPoints);
    for (int i = 0; i < 4; ++i) {
        cv::line(frame, ResizePoint(rectPoints[i]),
                 ResizePoint(rectPoints[(i + 1) % 4]), color);
    }
#if 0
    float angle = 0;//atan2(track.m_velocity[0], track.m_velocity[1]);
    cv::RotatedRect rr(track.m_rrect.center,
                       cv::Size2f(std::max(frame.rows / 20.f, static_cast<track_t>(3.f * fabs(track.m_velocity[0]))),
                       std::max(frame.cols / 20.f, static_cast<track_t>(3.f * fabs(track.m_velocity[1])))),
            180.f * angle / CV_PI);
    cv::ellipse(frame, rr, cv::Scalar(100, 100, 100), 2);
#endif
    if (drawTrajectory) {
        cv::Scalar cl = m_colors[track.m_ID % m_colors.size()];

        for (size_t j = 0; j < track.m_trace.size() - 1; ++j) {
            const TrajectoryPoint& pt1 = track.m_trace.at(j);
            const TrajectoryPoint& pt2 = track.m_trace.at(j + 1);
#if (CV_VERSION_MAJOR >= 4)
            cv::line(frame, ResizePoint(pt1.m_prediction),
                     ResizePoint(pt2.m_prediction), cl, 3, cv::LINE_AA);
#else
            cv::line(frame, ResizePoint(pt1.m_prediction),
                     ResizePoint(pt2.m_prediction), cl, 3, CV_AA);
#endif
            if (!pt2.m_hasRaw) {
#if (CV_VERSION_MAJOR >= 4)
                cv::circle(frame, ResizePoint(pt2.m_prediction), 4, cl, 3,
                           cv::LINE_AA);
#else
                cv::circle(frame, ResizePoint(pt2.m_prediction), 4, cl, 3,
                           CV_AA);
#endif
            }
        }
    }
}

///
/// \brief InitTracker
/// \param frame
/// \return
///
bool ObjectDetectorWithTracking::initDetector(cv::UMat frame) {
    // Initialize object detector
    object_detector = std::make_shared<ObjectDetector>(
        SMARTCAM_OBJECT_DETECTION_MODEL,
        SMARTCAM_OBJECT_DETECTION_TENSORRT_PLAN);

    return true;
}

///
/// \brief InitTracker
/// \param frame
/// \return
///
bool ObjectDetectorWithTracking::initTracker(cv::UMat frame) {
    TrackerSettings settings;
    settings.SetDistance(tracking::DistJaccard);
    settings.m_kalmanType = tracking::KalmanUnscented;
    settings.m_filterGoal = tracking::FilterRect;
    settings.m_lostTrackType =
        tracking::TrackCSRT;  // Use visual objects tracker for collisions
                              // resolving
    settings.m_matchType = tracking::MatchHungrian;
    settings.m_dt = 0.1f;             // Delta time for Kalman filter
    settings.m_accelNoiseMag = 0.3f;  // Accel noise magnitude for Kalman filter
    settings.m_distThres =
        0.5f;  // Distance threshold between region and object on two frames
    settings.m_minAreaRadius = frame.rows / 20.f;
    // settings.m_maximumAllowedSkippedFrames =
    //     cvRound(m_fps / 5);  // Maximum allowed skipped frames
    settings.m_maximumAllowedSkippedFrames = 1;
    settings.m_maxTraceLength = cvRound(5 * m_fps);  // Maximum trace length

    m_tracker = std::make_unique<CTracker>(settings);

    return true;
}

regions_t ObjectDetectorWithTracking::convertDetectionsToRegions(
    const std::vector<Detection>& detections) {
    regions_t regs;

    for (size_t i = 0; i < detections.size(); ++i) {
        struct Box b = detections[i].bbox;
        float confidence = detections[i].prob;
        cv::Rect bbox = cv::Rect(b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1);
        CRegion reg(bbox, ctdet::className[detections[i].classId], confidence);
        regs.push_back(reg);
    }

    return regs;
}