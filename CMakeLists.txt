cmake_minimum_required(VERSION 3.10)
project(semantic_rover)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(depthai CONFIG REQUIRED)

include_directories(include)

add_executable(log_data
  apps/log_data.cpp
  src/perception/oak_interface.cpp
  src/logger/data_logger.cpp
)

target_link_libraries(log_data
  ${OpenCV_LIBS}
  depthai::core
)
