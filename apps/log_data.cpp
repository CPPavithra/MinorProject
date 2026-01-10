#include "perception/oak_interface.hpp"
#include "logger/data_logger.hpp"
#include <iostream>
using namespace std;
int main()
{
  OakInterface oak;
  if(!oak,start())
  {
    cerr<<"Failed to start oak device\n";
    return -1;
  }

  DataLogger logger("data");
  int frame_id=0;

  while(true)
  {
    FrameData frame;
    if(!oak.getFrame(frame))
    {
      continue;
    }
    logger.logFrame(frame, frame_id++);
  }
}
