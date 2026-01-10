#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct Detection
{
  std::string label;
  float confidence;
  float x,y,z;
};

struct FrameData
{
  double timestamp;
  cv::Mat rgb;
  cv::Mat depth;
  std::vector<Detection>detections;
};

class OakInterface 
{
  public:
    OakInterface();
    bool start();
    bool getFrame(FrameData &frame);

  private:
    //internal handles
};
