#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <atomic>

#include "object_detector.h"
#include "ctdet_utils.h"
#include "Ctracker.h"
#include "ctdetConfig.h"

///
/// \brief The ObjectDetectorWithTracking class
///
class ObjectDetectorWithTracking
{
public:
    ObjectDetectorWithTracking();
    ObjectDetectorWithTracking(const ObjectDetectorWithTracking&) = delete;
    ObjectDetectorWithTracking(ObjectDetectorWithTracking&&) = delete;
    ObjectDetectorWithTracking& operator=(const ObjectDetectorWithTracking&) = delete;
    ObjectDetectorWithTracking& operator=(ObjectDetectorWithTracking&&) = delete;

    virtual ~ObjectDetectorWithTracking() = default;

    int runDetectAndTrack(const cv::Mat &img, std::vector<TrackingObject> &tracks);

    void drawTrack(cv::Mat frame, int resizeCoeff, const TrackingObject& track, bool drawTrajectory = true);
    

protected:
    std::shared_ptr<ObjectDetector> object_detector;
    std::unique_ptr<CTracker> m_tracker;

    bool m_showLogs;
    int m_fps;

    int m_captureTimeOut = 60000;
    int m_trackingTimeOut = 60000;

    static void CaptureAndDetect(ObjectDetectorWithTracking* thisPtr, std::atomic<bool>& stopCapture);

    bool initDetector(cv::UMat frame);
    bool initTracker(cv::UMat frame);

    void detection(const cv::Mat &frame, regions_t& regions);
    void tracking(const cv::Mat &frame, const regions_t& regions);

    regions_t convertDetectionsToRegions(const std::vector<Detection> &detections);

private:
    bool m_isTrackerInitialized = false;
    bool m_isDetectorInitialized = false;

    struct FrameInfo
    {
        cv::Mat m_frame;
        regions_t m_regions;
        std::condition_variable m_cond;
        std::mutex m_mutex;
    };
    FrameInfo m_frameInfo[2];

    std::vector<cv::Scalar> m_colors;

};
