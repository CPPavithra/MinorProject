#include "visualization/rerun_viz.hpp"
#include <sstream>
#include <vector>
#include <array>
#include <opencv2/imgproc.hpp>
#include <rerun/components/media_type.hpp>

// Ensure we have access to the Vec2D type
#include <rerun/datatypes/vec2d.hpp> 

RerunViz::RerunViz() : rec_("oak_semantics_demo") {
    rec_.spawn().exit_on_failure();
}

void RerunViz::logFrame(const FrameData& frame) {

    // ---------- RGB ----------
    if(!frame.rgb.empty()) {
        cv::Mat rgb_converted;
        cv::cvtColor(frame.rgb, rgb_converted, cv::COLOR_BGR2RGB);

        rec_.log(
            "oak/rgb",
            rerun::Image(
                rgb_converted.data, 
                {static_cast<uint32_t>(rgb_converted.cols), static_cast<uint32_t>(rgb_converted.rows)}, 
                rerun::datatypes::ColorModel::RGB
            )
        );
    }

    // ---------- DEPTH ----------
    if(!frame.depth.empty()) {
        rec_.log(
            "oak/depth",
            rerun::DepthImage(
                reinterpret_cast<const uint16_t*>(frame.depth.data),
                {static_cast<uint32_t>(frame.depth.cols), static_cast<uint32_t>(frame.depth.rows)}
            ).with_meter(1000.0) 
        );
    }

    // ---------- 2D BOXES ----------
    std::vector<rerun::datatypes::Vec2D> mins;
    std::vector<rerun::datatypes::Vec2D> sizes; 
    std::vector<std::string> labels;

    for(const auto& d : frame.detections) {
        mins.push_back({static_cast<float>(d.xmin), static_cast<float>(d.ymin)});
        
        sizes.push_back({
            static_cast<float>(d.xmax - d.xmin), 
            static_cast<float>(d.ymax - d.ymin)
        });

        std::stringstream conf_ss;
        conf_ss.precision(2);
        conf_ss << std::fixed << d.confidence;
        labels.push_back(d.label + " " + conf_ss.str());
    }

    if(!mins.empty()) {
        rec_.log(
            "oak/rgb/boxes",
            rerun::Boxes2D::from_mins_and_sizes(mins, sizes)
                .with_labels(labels)
                .with_class_ids(std::vector<uint16_t>(mins.size(), 1))
        );
    }

    // ---------- 3D POINTS ----------
    std::vector<rerun::Position3D> pts;

    for(const auto& d : frame.detections) {
        pts.push_back({
            static_cast<float>(d.x) / 1000.0f, 
            static_cast<float>(d.y) / 1000.0f, 
            static_cast<float>(d.z) / 1000.0f
        });
    }

    if(!pts.empty()) {
        rec_.log(
            "oak/detections_3d",
            rerun::Points3D(pts).with_radii(0.05f)
        );
    }

    // ---------- TEXT PANEL ----------
    std::stringstream ss;
    ss << "# Detections\n\n"; 

    for(const auto& d : frame.detections) {
        ss << "* **" << d.label << "**"
           << " (conf: " << d.confidence << ")\n"
           << "  * z: " << d.z << " mm\n";
    }

    // FIX IS HERE: components::MediaType instead of datatypes::MediaType
    rec_.log("oak/info", rerun::TextDocument(ss.str())
        .with_media_type(rerun::components::MediaType::markdown()));
}
