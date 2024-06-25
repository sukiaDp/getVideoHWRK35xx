//
// Created by cat on 6/25/24.
//

#include "App.h"

#define SAVE_FRAME 3000

App::App() {
    cap.open(1, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] cap open Fail";
        exit(-1);
    }

    // 设置分辨率和帧率
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 2560);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1440);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 设置格式为 MJPG
    cap.set(cv::CAP_PROP_FPS, 15);

    frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    fps = cap.get(cv::CAP_PROP_FPS);

    std::string writePipeline = "appsrc ! "
                                "queue ! "
                                "videoconvert ! video/x-raw,format=NV12 ! "
                                "mpph264enc ! h264parse ! mp4mux ! "
                                "filesink location=output.mp4";
    writer.open(writePipeline,
                cv::CAP_GSTREAMER,
                0,
                fps,
                cv::Size(frameWidth, frameHeight),
                true);
    if (!writer.isOpened()) {
        std::cerr << "[ERROR] writer open Fail";
        exit(-1);
    }

    std::cout << "frame_width = " << frameWidth << "\nframe_height = " << frameHeight << "\nfps = " << fps << std::endl;

    pushNewFrame2Queue();
    if (imageQueue.front().empty()) {
        std::cerr << "[ERROR] read empty";
        exit(-1);
    }

    std::cout << "init Done, ready for process\n";
}

void App::run() {//初始化读取线程
    readThread = std::move(std::thread(&App::readLoop, this));
    //初始化处理线程
    writeThread = std::move(std::thread(&App::writeLoop, this));

    std::cout << "Thread working\n";
}

void App::readLoop() {
    size_t loopCount = 0;
    while (true) {
        auto startTime = std::chrono::system_clock::now();
        pushNewFrame2Queue();
        if (imageQueue.back().empty()) {
            return;
        }
        cv.notify_one();

        //终止条件
        if (++loopCount > SAVE_FRAME) {
            std::cout << "push done";
            jobDone = true;
            cv.notify_all();
            break;
        } else {
            std::cout << "push " << loopCount << "frame\n";
        }
        std::cout << "push used time = "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count()<< std::endl
                  << "queue size = " << imageQueue.size() << std::endl;
    }
    cap.release();
}

void App::pushNewFrame2Queue() {
    cv::Mat img(frameWidth, frameHeight, CV_8UC3);
    cap.read(img);

    std::lock_guard<std::mutex> lock(queueMtx);
    imageQueue.push(std::move(img));
}

void App::writeLoop() {
    static size_t loopTime = 0;
    while (true) {
        if (imageQueue.size() > 1) {
            std::lock_guard<std::mutex> lock(queueMtx);
            writer.write(imageQueue.front());
            imageQueue.pop();
            std::cout << "save " << ++loopTime << "frame\n";
        } else {
            if (jobDone) {
                break;
            }

            std::unique_lock<std::mutex> lock(waitMtx);
            cv.wait(lock);
        }
    }
    writer.release();
}

void App::waitForThreads() {
    if (readThread.joinable()) {
        readThread.join();
    }
    if (writeThread.joinable()) {
        writeThread.join();
    }
}

App::~App() {
    waitForThreads();
};
