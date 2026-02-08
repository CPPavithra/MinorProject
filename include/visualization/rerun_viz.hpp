#pragma once

#include <rerun.hpp>
#include "logger/data_logger.hpp"

class RerunViz {
public:
    RerunViz();
    void logFrame(const FrameData& frame);

private:
    rerun::RecordingStream rec_;
};
