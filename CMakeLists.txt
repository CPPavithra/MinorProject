cmake_minimum_required(VERSION 3.10)
project(miniproject)

set(CMAKE_CXX_STANDARD 17)

# 1. TELL CMAKE WHERE TO LOOK (From your old project)
# This was missing. It tells CMake to check /usr/local/lib for the config files.
list(APPEND CMAKE_PREFIX_PATH "/usr/local/lib/cmake/rerun_sdk")

find_package(OpenCV REQUIRED)
find_package(depthai CONFIG REQUIRED)
# 2. FIND THE PACKAGE (From your old project)
# You were trying to link 'rerun_sdk' below without finding it first.
find_package(rerun_sdk REQUIRED)

add_executable(log_data
    apps/log_data.cpp
    src/perception/oak_interface.cpp
    src/logger/data_logger.cpp
    src/visualization/rerun_viz.cpp
)

target_include_directories(log_data
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    # 3. INCLUDE DIRECTORIES (From your old project)
    # Sometimes the target interface doesn't propagate these automatically depending on install.
    "/usr/local/include/rerun"
)

target_link_libraries(log_data
    PRIVATE
    depthai::core
    ${OpenCV_LIBS}
    rerun_sdk
)
