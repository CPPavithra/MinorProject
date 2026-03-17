#pragma once

#include <rerun.hpp>
#include <vector>
#include "perception/oak_interface.hpp" 
#include "perception/slam_manager.hpp"  

class RerunViz {
public:
    RerunViz();
    void logFrame(const FrameData& frame);
    void logPose(const RoverPose& pose);

private:
    rerun::RecordingStream rec_;
    
    // CHANGE THIS LINE: Use Vec3D for the trajectory
    std::vector<rerun::datatypes::Vec3D> trajectory_; 
};
