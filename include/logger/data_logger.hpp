#ifndef DATA_LOGGER_HPP
#define DATA_LOGGER_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include "perception/oak_interface.hpp"

class DataLogger {
public:
    explicit DataLogger(const std::string& base_path);
    void logFrame(const FrameData& frame, int frame_id);

private:
    std::string base_path;
};

#endif
