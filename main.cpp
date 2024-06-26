//
// Created by cat on 6/19/24.
//
#include "App.h"
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
    App app;
    auto startTime = std::chrono::system_clock::now();
    app.run();

    app.waitForThreads();
    std::cout << "\n\n\njob Done, time spend :" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() << "ms\n";
}
/*int main()
{
    cv::VideoCapture cap(1, cv::CAP_V4L2); // 使用 V4L2 后端

    // 设置分辨率和帧率
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 设置格式为 MJPG
    cap.set(cv::CAP_PROP_FPS, 30);

    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);

    std::string writePipeline =  "appsrc ! "
                                 "queue ! "
                                 "videoconvert ! video/x-raw,format=NV12 ! "
                                 "mpph264enc ! h264parse ! mp4mux ! "
                                 "filesink location=output.mp4";
    cv::VideoWriter writer(writePipeline,
                           cv::CAP_GSTREAMER,
                           0,
                           fps,
                           cv::Size(frameWidth, frameHeight),
                           true);

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

    // 预读取一张图片
    cap >> *lastFramePtr;

    for(size_t count = 0; count < 3000; count++)
    {
        auto tStart = std::chrono::system_clock::now();
        cap >> *curFramePtr;
        auto tRead = std::chrono::system_clock::now() - tStart;
        if(curFramePtr->empty())
        {
            std::cerr << "[warn]empty frame";
            continue;
        }

        auto diffValue =  getDiffValue(curFramePtr, lastFramePtr);
        std::cout << "diff percent:" << diffValue << std::endl;
        cv::putText(*curFramePtr,
                    "diff count = " + std::to_string(diffValue),
                    cv::Point(50, 50),
                    cv::FONT_HERSHEY_SIMPLEX,
                    2.0,
                    cv::Scalar(0, 0, 255),
                    3);

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
}*/