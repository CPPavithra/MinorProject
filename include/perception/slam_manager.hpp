#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

// Forward declaration so we don't leak Stella headers everywhere
namespace stella_vslam { class system; }

// A simple struct to hold our global rover position
struct RoverPose {
    float x, y, z;
    bool valid;
};

class SlamManager {
public:
    SlamManager();
    ~SlamManager();

    bool init(const std::string& config_path, const std::string& vocab_path);
    RoverPose trackStereo(const cv::Mat& left, const cv::Mat& right, double timestamp);
    void stop();

private:
    std::shared_ptr<stella_vslam::system> slam_;
};
