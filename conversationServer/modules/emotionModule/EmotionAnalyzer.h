#pragma once
#include "core/IAnalyzer.h"
#include "core/IEmotionAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <fstream>
class EmotionAnalyzer  : public IEmotionAnalyzer{
private:
    cv::dnn::Net net;
    std::vector<std::string> emotions;
    void loadClasses(const std::string& classesPath);
public:
    EmotionAnalyzer(const std::string& modelPath, const std::string& classesPath) : net(cv::dnn::readNetFromONNX(modelPath)) {
        this->loadClasses(classesPath);
    }
    std::string analyze(const cv::Mat& frame);
};
