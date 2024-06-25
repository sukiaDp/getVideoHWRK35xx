cmake_minimum_required(VERSION 3.10)
project(videoGetHW)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置Boost库路径
# set(BOOST_ROOT "/usr/local/boost")

# 找到Boost库
find_package(Boost 1.81.0 REQUIRED COMPONENTS json)
find_package(OpenCV REQUIRED)

# 包含Boost和OpenCV库头文件
include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

# 添加可执行文件
add_executable(videoGetHW main.cpp
        main.cpp
        App.cpp
        App.h)

# 链接Boost和OpenCV库
target_link_libraries(videoGetHW ${Boost_LIBRARIES} ${OpenCV_LIBS} pthread)
