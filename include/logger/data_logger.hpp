#pragma once

#include "perception/oak_interface.hpp"
#include <string>

class DataLogger
{
  public:
    DataLogger(const std::string& base_path); 
    void logFrame(const FrameData& frame, int frame_id);
  private:
    std::string base_path;
};
