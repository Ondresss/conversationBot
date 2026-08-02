#pragma once
#include "../core/IPointsOfInterestAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>

class YoloAnalyzer : public IPointsOfInterestAnalyzer {
public:
    struct YoloParams {
        float inputWidth = 640.0f;
        float inputHeight = 640.0f;
        float scoreThreshold = 0.25f;
        float nmsThreshold = 0.45f;
        float confidenceThreshold = 0.25f;
        std::string cocoClassesFile;
        std::string netModelPath;
    };

    YoloAnalyzer(const YoloParams& params);
    std::vector<PointOfInterest> analyze(const cv::Mat& frame) override;

private:
    cv::dnn::Net net;
    std::vector<std::string> cocoClasses;
    YoloParams params;

    void loadCocoClasses(const std::string& cocoClassesFile);
    void load_net(cv::dnn::Net &net);
    cv::Mat preprocess(const cv::Mat& frame);
};
