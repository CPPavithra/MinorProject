#include "perception/oak_interface.hpp"
#include "logger/data_logger.hpp"
#include "perception/slam_manager.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "visualization/rerun_viz.hpp"

using namespace std;

int main()
{
  OakInterface oak;
  std::cout << "Starting OAK device..." << std::endl;

  if(!oak.start())
  {
    cerr << "Failed to start oak device\n";
    return -1;
  }
  std::cout << "OAK started. Initializing logger..." << std::endl;

  SlamManager slam;
  if(!slam.init("slam_assets/oak_stereo.yaml", "slam_assets/orb_vocab.fbow")) {
      std::cerr << "SLAM Failed to start. Check file paths!" << std::endl;
      return -1;
  }

  DataLogger logger("data");
  RerunViz viz;
  int frame_id = 0;
  
  std::cout << "Waiting for first frame (this may take 1-2 mins for model compilation)..." << std::endl;

  while(true)
  {
    FrameData frame;
    // If getFrame returns false, it means data isn't ready yet
    if(!oak.getFrame(frame))
    {
      // Print a dot to show it's alive, then wait 100ms
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    
    std::cout << "\nLogging frame " << frame_id << std::endl;

    // 1. Run SLAM
    RoverPose pose = slam.trackStereo(frame.left, frame.right, frame.timestamp);
    
    // 2. Store pose in the frame for the logger
    frame.pose_valid = pose.valid;
    frame.pose_x = pose.x;
    frame.pose_y = pose.y;
    frame.pose_z = pose.z;

    // 3. Print & Visualize
    if(pose.valid) {
        viz.logPose(pose);
        std::cout << "  -> SLAM Pose [X: " << pose.x << " | Y: " << pose.y << " | Z: " << pose.z << "]" << std::endl;
    } else {
        std::cout << "  -> SLAM Pose [Initializing / Tracking Lost]" << std::endl;
    }

    // 4. Log to disk and Rerun
    logger.logFrame(frame, frame_id);
    viz.logFrame(frame);
    
    frame_id++;
    
    // Stop after 500 frames for testing (you can change this limit)
    if (frame_id > 500) break;
  }
  
  // Clean shutdown
  std::cout << "Stopping system..." << std::endl;
  slam.stop();
  oak.stop(); 
  
  return 0;
}
