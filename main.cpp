//
// Created by cat on 6/19/24.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/ocl.hpp>
#include <chrono>
//#include <thread>

//#define GET_DIFF_VALUE_DEBUG_ACTIVE

double getDiffValue(cv::Mat *curImg, cv::Mat *lastImg)
{
    cv::Mat curImgGray;
    cv::Mat lastImgGray;
    cv::cvtColor(*curImg, curImgGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(*lastImg, lastImgGray, cv::COLOR_BGR2GRAY);

    cv::Mat diff;
    cv::absdiff(curImgGray, lastImgGray, diff);

#ifdef GET_DIFF_VALUE_DEBUG_ACTIVE
    auto diffDisp = diff.clone();
#endif

    diff = diff > 5;
    auto diffCount = cv::countNonZero(diff);

#ifdef GET_DIFF_VALUE_DEBUG_ACTIVE
    std::cout << "diff count = " << diffCount << std::endl;
    cv::putText(diffDisp,
                "diff count = " + std::to_string((float)diffCount / ((float)curImg->cols * (float)curImg->rows)),
                cv::Point(50, 50),
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(255, 255, 255));
    cv::imshow("diff", diffDisp);
    cv::waitKey(1);
#endif

    return (float)diffCount / ((float)curImg->cols * (float)curImg->rows);
}

int main()
{
    std::string readPipeline = "v4l2src device=/dev/video1 ! "
                               "image/jpeg,width=2560,height=1440,framerate=15/1 ! "
                               "jpegdec ! videoconvert ! appsink";
    cv::VideoCapture cap(readPipeline, cv::CAP_GSTREAMER);

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
    cv::Mat frameBuffer[2];

    cv::Mat *curFramePtr = &frameBuffer[0];
    cv::Mat *lastFramePtr = &frameBuffer[1];

    // 预读取一张图片作为底
    cap >> *lastFramePtr;

    for(size_t count = 0; count < 3000; count ++)
    {
        auto tStart = std::chrono::system_clock::now();
        cap >> *curFramePtr;
        auto tRead = std::chrono::system_clock::now() - tStart;
        if(curFramePtr->empty())
        {
            std::cerr << "[warn]empty frame";
            continue;
        }

        std::cout << "diff percent:" << getDiffValue(curFramePtr, lastFramePtr) << std::endl;

        writer.write(*curFramePtr);
        auto tWrite = std::chrono::system_clock::now() - tStart - tRead;

        std::cout << "frame.rows = " << curFramePtr->rows << "\nframe.cols = " << curFramePtr->cols << std::endl;
        std::cout << "_________V0_______" << count << "_________________\n";
        std::cout << "Read cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(tRead).count() << "ms\n";
        std::cout << "Write cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(tWrite).count() << "ms\n";

        auto exchangePtr = lastFramePtr;
        lastFramePtr = curFramePtr;
        curFramePtr = exchangePtr;
    }


    std::cout << "total cost:" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() << "ms\n";
    writer.release();
    cap.release();

    std::cout << "done";
    return 0;
}