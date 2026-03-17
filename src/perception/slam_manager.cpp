#include "perception/slam_manager.hpp"
#include <stella_vslam/system.h>
#include <stella_vslam/config.h>
#include <Eigen/Dense>
#include <iostream>

SlamManager::SlamManager() {}

SlamManager::~SlamManager() { 
    stop(); 
}

bool SlamManager::init(const std::string& config_path, const std::string& vocab_path) {
    try {
        std::cout << "Initializing Stella V-SLAM..." << std::endl;
        auto cfg = std::make_shared<stella_vslam::config>(config_path);
        slam_ = std::make_shared<stella_vslam::system>(cfg, vocab_path);
        slam_->startup();
        std::cout << "Stella V-SLAM Initialized Successfully!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "SLAM Init Failed: " << e.what() << std::endl;
        return false;
    }
}

RoverPose SlamManager::trackStereo(const cv::Mat& left, const cv::Mat& right, double timestamp) {
    RoverPose pose{0.0f, 0.0f, 0.0f, false};
    if(!slam_) return pose;

    // Feed the left/right grayscale images to the math engine
    // It returns a shared_ptr to the camera pose with respect to the World
    auto Tcw = slam_->feed_stereo_frame(left, right, timestamp);

    // SAFETY CHECK: If Tcw is null, tracking is lost or initializing.
    if (!Tcw) {
        return pose; // pose.valid is false
    }

    // Dereference the pointer (using ->) to do matrix math
    Eigen::Matrix4d Twc = Tcw->inverse();

    // Extract X, Y, Z translation
    pose.x = Twc(0, 3);
    pose.y = Twc(1, 3);
    pose.z = Twc(2, 3);
    pose.valid = true;

    return pose;
}
void SlamManager::stop() {
    if(slam_) {
        std::cout << "Shutting down SLAM engine..." << std::endl;
        slam_->shutdown();
        slam_.reset();
    }
}
