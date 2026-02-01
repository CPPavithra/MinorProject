#include <nlohmann/json.hpp>
#include "logger/data_logger.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

DataLogger::DataLogger(const std::string& base_path)
    : base_path(base_path) {

    fs::create_directories(base_path + "/rgb");
    fs::create_directories(base_path + "/depth");
    fs::create_directories(base_path + "/semantics");

    std::cout << "DataLogger initialized at: " << base_path << std::endl;
}


void DataLogger::logFrame(const FrameData& frame, int frame_id) {

    // ---------- RGB ----------
    if(!frame.rgb.empty()) {
        std::string rgb_path =
            base_path + "/rgb/frame_" + std::to_string(frame_id) + ".png";
        cv::imwrite(rgb_path, frame.rgb);
    }

    // ---------- DEPTH ----------
    if(!frame.depth.empty()) {
        cv::Mat depth_vis;

        cv::normalize(frame.depth, depth_vis, 0, 255,
                      cv::NORM_MINMAX, CV_8UC1);

        std::string depth_path =
            base_path + "/depth/frame_" + std::to_string(frame_id) + ".png";
        cv::imwrite(depth_path, depth_vis);
    }

    // ---------- SEMANTICS (JSON) ----------
    json j;
    j["timestamp"] = frame.timestamp;
    j["detections"] = json::array();

    for(const auto& det : frame.detections) {
        j["detections"].push_back({
            {"label", det.label},
            {"confidence", det.confidence},
            {"x", det.x},
            {"y", det.y},
            {"z", det.z}
        });
    }

    std::string json_path =
        base_path + "/semantics/frame_" + std::to_string(frame_id) + ".json";

    std::ofstream out(json_path);
    out << j.dump(2);   // pretty printed

    std::cout << "Saved frame " << frame_id << std::endl;
}

