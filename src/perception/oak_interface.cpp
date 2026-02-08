#include "perception/oak_interface.hpp"

bool OakInterface::start() {
    // -------------------------------------------------------------------------
    // 1. SETUP CAMERAS
    // -------------------------------------------------------------------------
    auto colorCam = pipeline_.create<dai::node::Camera>();
    // Note: The build() parameters usually define resolution/FPS in your wrapper.
    // If you need specific YOLO resolution (640x640), ensure your wrapper's 
    // build() or defaults support it, or we rely on the NN scaling.
    colorCam->build(dai::CameraBoardSocket::CAM_A);

    auto monoLeft = pipeline_.create<dai::node::Camera>();
    monoLeft->build(dai::CameraBoardSocket::CAM_B);

    auto monoRight = pipeline_.create<dai::node::Camera>();
    monoRight->build(dai::CameraBoardSocket::CAM_C);

    // -------------------------------------------------------------------------
    // 2. SETUP STEREO
    // -------------------------------------------------------------------------
    auto stereo = pipeline_.create<dai::node::StereoDepth>();
    stereo->setDefaultProfilePreset(dai::node::StereoDepth::PresetMode::DEFAULT);
    stereo->setLeftRightCheck(true);
    stereo->setExtendedDisparity(false);
    stereo->setSubpixel(false);
    
    // -------------------------------------------------------------------------
    // 3. SETUP SPATIAL DETECTION (SEMANTICS)
    // -------------------------------------------------------------------------
    auto spatialDet = pipeline_.create<dai::node::SpatialDetectionNetwork>();
    
    spatialDet->input.setBlocking(false);
    spatialDet->setBoundingBoxScaleFactor(0.5f);
    spatialDet->setDepthLowerThreshold(100);
    spatialDet->setDepthUpperThreshold(5000);

    // Define Model
    dai::NNModelDescription modelDesc;
    modelDesc.model = "yolov6-nano"; 
    
    // Build NN using the helper from your example
    // This links Camera -> NN and Stereo -> NN internally
    spatialDet->build(colorCam, stereo, modelDesc, 30);

    // -------------------------------------------------------------------------
    // 4. MANUAL LINKING (Required for Stereo)
    // -------------------------------------------------------------------------
    // Link Mono Cameras to Stereo inputs
    auto leftOut  = monoLeft->requestOutput({640, 400});
    auto rightOut = monoRight->requestOutput({640, 400});
    
    leftOut->link(stereo->left);
    rightOut->link(stereo->right);

    // -------------------------------------------------------------------------
    // 5. QUEUES
    // -------------------------------------------------------------------------
    // In your wrapper, createOutputQueue() creates the XLink implicitly.
    // We access the 'passthrough' outputs to get the frames synced with detections.
    
    rgbQueue_   = spatialDet->passthrough.createOutputQueue();
    depthQueue_ = spatialDet->passthroughDepth.createOutputQueue();
    detQueue_   = spatialDet->out.createOutputQueue();

    // Save classes if available
    if(spatialDet->getClasses().has_value()) {
        labelMap_ = spatialDet->getClasses().value();
    }

    // -------------------------------------------------------------------------
    // 6. START
    // -------------------------------------------------------------------------
    pipeline_.start();
    
    return true;
}

bool OakInterface::getFrame(FrameData& frame) {
    if(!rgbQueue_ || !depthQueue_ || !detQueue_) {
        return false;
    }

    // CHANGE: Use get() instead of tryGet(). 
    // This will BLOCK (wait) until the Neural Network produces a result.
    // Since we use 'passthrough', the RGB and Depth will be perfectly synced to this detection.
    
    auto detMsg   = detQueue_->get<dai::SpatialImgDetections>();
    auto rgbMsg   = rgbQueue_->get<dai::ImgFrame>();
    auto depthMsg = depthQueue_->get<dai::ImgFrame>();

    // If pointers are null (shouldn't happen with get() unless pipeline stopped)
    if(!rgbMsg || !depthMsg || !detMsg) {
        return false;
    }

    // ---------- RGB + Depth ----------
    frame.rgb   = rgbMsg->getCvFrame();
    frame.depth = depthMsg->getCvFrame();

    frame.timestamp = std::chrono::duration<double>(
        rgbMsg->getTimestamp().time_since_epoch()
    ).count();

    frame.detections.clear();

for(const auto& d : detMsg->detections) {
    Detection det;

    if (!labelMap_.empty() && d.label < labelMap_.size()) {
        det.label = labelMap_[d.label];
    } else {
        det.label = std::to_string(d.label);
    }

    det.confidence = d.confidence;

    // -------- 3D spatial ----------
    det.x = d.spatialCoordinates.x;
    det.y = d.spatialCoordinates.y;
    det.z = d.spatialCoordinates.z;

    // -------- 2D bounding box ----------
    det.xmin = d.xmin;
    det.ymin = d.ymin;
    det.xmax = d.xmax;
    det.ymax = d.ymax;

    frame.detections.push_back(det);
}

    return true;
}
void OakInterface::stop() {
    if(pipeline_.isRunning()) {
        pipeline_.stop();
    }
}
