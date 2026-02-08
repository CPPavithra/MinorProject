#include "visualization/rerun_viz.hpp"
#include <sstream>
#include <vector>
#include <array>
#include <cmath>
#include <opencv2/imgproc.hpp>

// --- RERUN INCLUDES ---
// Core archetypes
#include <rerun/archetypes/pinhole.hpp>
#include <rerun/archetypes/boxes3d.hpp>
#include <rerun/archetypes/boxes2d.hpp>
#include <rerun/archetypes/image.hpp>
#include <rerun/archetypes/depth_image.hpp>
#include <rerun/archetypes/points2d.hpp> // For Tactical Map
#include <rerun/archetypes/scalar.hpp>   // For Analytics Graph

// Components
#include <rerun/components/media_type.hpp>
#include <rerun/components/half_size3d.hpp>
#include <rerun/components/position3d.hpp>
#include <rerun/components/text.hpp>
#include <rerun/components/color.hpp>

// Datatypes
#include <rerun/datatypes/vec2d.hpp>

RerunViz::RerunViz() : rec_("oak_semantics_demo") {
    rec_.spawn().exit_on_failure();
}

void RerunViz::logFrame(const FrameData& frame) {
    
    // Safety check
    if(frame.rgb.empty()) return;

    // =========================================================
    // 1. SETUP CAMERA INTRINSICS (The "Pinhole")
    // =========================================================
    float width = static_cast<float>(frame.rgb.cols);
    float height = static_cast<float>(frame.rgb.rows);
    
    rec_.log(
        "oak", 
        rerun::Pinhole::from_focal_length_and_resolution(
            800.0f, // Approx focal length for OAK-D.
            {width, height}
        )
    );

    // =========================================================
    // 2. RGB IMAGE
    // =========================================================
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

    // =========================================================
    // 3. DEPTH IMAGE (Automatic 3D Point Cloud)
    // =========================================================
    if(!frame.depth.empty()) {
        rec_.log(
            "oak/depth",
            rerun::DepthImage(
                reinterpret_cast<const uint16_t*>(frame.depth.data),
                {static_cast<uint32_t>(frame.depth.cols), static_cast<uint32_t>(frame.depth.rows)}
            ).with_meter(1000.0) 
        );
    }

    // =========================================================
    // 4. 2D BOXES (Overlay on the RGB Image)
    // =========================================================
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

    // =========================================================
    // 5. 3D BOXES (The "Semantic SLAM" Nodes)
    // =========================================================
    std::vector<rerun::components::Position3D> centers;
    std::vector<rerun::components::HalfSize3D> half_sizes; 
    std::vector<rerun::components::Text> labels3d;
    std::vector<rerun::components::Color> colors;

    // Vectors for the Tactical Map (Step 6)
    std::vector<rerun::datatypes::Vec2D> map_points;
    std::vector<rerun::components::Color> map_colors;
    std::vector<std::string> map_labels;

    for(const auto& d : frame.detections) {
        // Convert mm to meters
        float x_m = static_cast<float>(d.x) / 1000.0f;
        float y_m = static_cast<float>(d.y) / 1000.0f;
        float z_m = static_cast<float>(d.z) / 1000.0f;

        // --- 3D Visualization Data ---
        centers.push_back({x_m, y_m, z_m});
        half_sizes.push_back({0.1f, 0.1f, 0.1f}); // 20cm fixed box
        
        std::stringstream label_ss;
        label_ss << d.label << " (" << (int)d.z << "mm)";
        labels3d.push_back(label_ss.str());

        // Color Logic
        rerun::components::Color c;
        if(d.label == "person") c = {0xFF0000FF};      // Red
        else if(d.label == "bottle") c = {0x0000FFFF}; // Blue
        else c = {0x00FF00FF};                         // Green
        colors.push_back(c);

        // --- Tactical Map Data (Top Down: X vs Z) ---
        // Note: In OAK, Z is forward. We map OAK's X to Map X, and OAK's Z to Map Y (Up).
        map_points.push_back({x_m, z_m}); 
        map_colors.push_back(c);
        map_labels.push_back(d.label);

        // --- Live Analytics Data ---
        std::string path = "dashboard/confidence/" + d.label;
        rec_.log(path, rerun::Scalar(d.confidence));
    }

    if(!centers.empty()) {
        rec_.log(
            "oak/semantics", 
            rerun::Boxes3D::from_centers_and_half_sizes(centers, half_sizes)
                .with_labels(labels3d)
                .with_colors(colors)
        );
    }

    // =========================================================
    // 6. HIGH LEVEL: TACTICAL MAP (Top-Down 2D View)
    // =========================================================
    // Log the "Robot" at (0,0)
    rec_.log("planning/map/robot", 
        rerun::Points2D({{0.0f, 0.0f}})
            .with_colors({{0xFFFFFFFF}}) // White dot
            .with_radii({0.05f})
            .with_labels({"Me"})
    );

    if(!map_points.empty()) {
        rec_.log("planning/map/objects", 
            rerun::Points2D(map_points)
                .with_colors(map_colors)
                .with_labels(map_labels)
                .with_radii({0.1f}) 
        );
    }

    // =========================================================
    // 7. TEXT LOGGING
    // =========================================================
    std::stringstream ss;
    ss << "# Detections\n\n"; 

    for(const auto& d : frame.detections) {
        ss << "* **" << d.label << "**"
           << " (conf: " << d.confidence << ")\n"
           << "  * z: " << d.z << " mm\n";
    }

    rec_.log("oak/info", rerun::TextDocument(ss.str())
        .with_media_type(rerun::components::MediaType::markdown()));
}
