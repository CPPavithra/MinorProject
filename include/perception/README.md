## DATA LOGGING SPECIFICATIONS BEFORE WE START

Each frame logs:
- Timestamp (seconds)
- RGB Image (BGR)
- Depth Image 
- Object detections:
  - label
  - confidence
  - 3D position of it in the camera frame

Data is stored as:
- PNG Images
- JSONL metadata

WHY JSONL?
- Streamable
- Easy to parse in C++,Python
- Append-only 

`oak_interface.cpp` ONLY-
1. Builds the DepthAI pipeline
2. Grabs RGB Data
3. Grabs Depth frame
4. Grabs spatial detections 
5. Fills the `FrameData`
