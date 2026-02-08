#ifndef OAK_INTERFACE_HPP
#define OAK_INTERFACE_HPP

#include <opencv2/opencv.hpp>
#include <depthai/depthai.hpp>
#include <vector>
#include <string>
#include <memory> // Required for shared_ptr

struct Detection {
    std::string label;
    float confidence;
    float x, y, z;
    float xmin, ymin;       // bbox
    float xmax, ymax;
};

struct FrameData {
    cv::Mat rgb;
    cv::Mat depth;
    double timestamp;
    std::vector<Detection> detections;
};

class OakInterface {
public:
    OakInterface() = default;
    ~OakInterface() = default;

    bool start();
    bool getFrame(FrameData& frame);
    void stop();

private:
    dai::Pipeline pipeline_;
    std::shared_ptr<dai::Device> device_;
    // Corrected types based on your hardware version
    std::shared_ptr<dai::MessageQueue> rgbQueue_; 
    std::shared_ptr<dai::MessageQueue> depthQueue_;
    std::shared_ptr<dai::MessageQueue> detQueue_;
    
    std::vector<std::string> labelMap_;
};

#endif // OAK_INTERFACE_HPP
