#include "perception/oak_interface.hpp"
#include "logger/data_logger.hpp"
#include <iostream>
#include <thread> // Add this for sleep
#include <chrono> // Add this for sleep
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
      // Print a dot to show it's alive, then wait 10ms
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    
    // Once we get a frame, print a new line
    std::cout << "\nLogging frame " << frame_id << std::endl;
   if(oak.getFrame(frame)) {
    logger.logFrame(frame, frame_id);
    viz.logFrame(frame);
    frame_id++;
   }
    if (frame_id > 100) break;
  }
  
  oak.stop(); // Good practice to stop explicitly
  return 0;
}
