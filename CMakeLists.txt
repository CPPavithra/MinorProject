cmake_minimum_required(VERSION 3.10)
project(miniproject)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(depthai CONFIG REQUIRED)

add_executable(log_data
    apps/log_data.cpp
    src/perception/oak_interface.cpp
    src/logger/data_logger.cpp
)

target_include_directories(log_data
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(log_data
    PRIVATE
    depthai::core
    ${OpenCV_LIBS}
)
