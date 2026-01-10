#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

namespace dai 
{
  class Device;
  class DataOutputQueue;
}

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
    std::unique_ptr<dai::Device> device_;
    std::shared_ptr<dai::DataOutputQueue> rgbQueue_;
    std::shared_ptr<dai::DataOutputQueue> depthQueue_;
    std::shared_ptr<dai::DataOutputQueue> detQueue_;
};
