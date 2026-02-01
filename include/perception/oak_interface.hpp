#ifndef OAK_INTERFACE_HPP
#define OAK_INTERFACE_HPP

#include <opencv2/opencv.hpp>
#include <depthai/depthai.hpp>


struct Detection {
    std::string label;
    float confidence;
    float x, y, z;
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
    std::shared_ptr<dai::MessageQueue> rgbQueue_; //for the newest version of depthai in my laptop 3.1.2
    std::shared_ptr<dai::MessageQueue> depthQueue_;
};

#endif // OAK_INTERFACE_HPP
