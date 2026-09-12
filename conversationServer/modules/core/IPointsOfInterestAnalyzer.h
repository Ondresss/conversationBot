#pragma once
#include <opencv2/opencv.hpp>
#include "PointOfInterest.h"
#include "core/IAnalyzer.h"

class IPointsOfInterestAnalyzer : public IAnalyzer {
public:
    virtual std::vector<PointOfInterest> analyze(const cv::Mat& image) = 0;
};
