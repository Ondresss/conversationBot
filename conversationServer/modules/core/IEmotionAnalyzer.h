#pragma once
#include "core/IAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <string>

class IEmotionAnalyzer : public IAnalyzer {
public:
    virtual std::string analyze(const cv::Mat& frame) = 0;
};
