//
// Created by cat on 6/25/24.
//

#ifndef VIDEOGETHW_APP_H
#define VIDEOGETHW_APP_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/ocl.hpp>
#include <chrono>
#include <thread>
#include <condition_variable>

class App {
public:
    App();

    virtual ~App();

    void waitForThreads();

    void run();
private:
    cv::VideoCapture cap;
    cv::VideoWriter writer;

    std::mutex waitMtx;
    std::mutex queueMtx;
    std::condition_variable cv;
    std::thread readThread;
    std::thread writeThread;
    bool jobDone = false;


    std::queue<cv::Mat> imageQueue;
    int frameWidth;
    int frameHeight;
    double fps;

    void pushNewFrame2Queue();

    void readLoop();

    void writeLoop();
};


#endif //VIDEOGETHW_APP_H
