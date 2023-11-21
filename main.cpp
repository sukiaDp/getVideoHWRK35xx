//
// Created by cat on 6/19/24.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/ocl.hpp>
#include <chrono>
#include <thread>

int main()
{
    std::string readPipeline = "v4l2src device=/dev/video1 ! "
                               "queue ! "
                               "image/jpeg,width=1920,height=1080,framerate=30/1 ! "
                               "jpegdec ! videoconvert ! appsink";
    cv::VideoCapture cap(readPipeline, cv::CAP_GSTREAMER);
    //cv::VideoCapture cap(1);

    std::cout << "cap init" << std::endl;
    // 获取视频帧的宽度和高度
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);

    std::string writePipeline =  "appsrc ! queue ! videoconvert ! video/x-raw,format=NV12 ! mpph264enc ! h264parse ! mp4mux ! filesink location=output.mp4";
    cv::VideoWriter writer(writePipeline,
                           cv::CAP_GSTREAMER,
                           0,
                           fps,
                           cv::Size(frameWidth, frameHeight),
                           true);

    //cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(frameWidth, frameHeight));

    std::cout << "frame_width = " << frameWidth << "\nframe_height = " << frameHeight << "\nfps = " << fps << std::endl;

    if(!cap.isOpened())
    {
        std::cerr << "[ERROR] cap open Fail";
        return -1;
    }

    if(!writer.isOpened())
    {
        std::cerr << "[ERROR] writer open Fail";
        return -1;
    }

    auto startTime = std::chrono::system_clock::now();
    std::cout << "init Done";
    cv::Mat frame;
    for(size_t count = 0; count < 3000; count ++)
    {
        auto tStart = std::chrono::system_clock::now();
        cap >> frame;
        auto tRead = std::chrono::system_clock::now() - tStart;
        if(frame.empty())
        {
            std::cerr << "[warn]empty frame";
            continue;
        }
        writer.write(frame);
        auto tWrite = std::chrono::system_clock::now() - tStart - tRead;

        std::cout << "frame.rows = " << frame.rows << "\nframe.cols = " << frame.cols << std::endl;
        std::cout << "_________V0_______" << count << "_________________\n";
        std::cout << "Read cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(tRead).count() << "ms\n";
        std::cout << "Write cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(tWrite).count() << "ms\n";
    }


    std::cout << "total cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() << "ms\n";
    writer.release();
    cap.release();

    std::cout << "done";
    return 0;
}