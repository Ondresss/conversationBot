#pragma once
#include <opencv2/opencv.hpp>
#include "PointOfInterest.h"

class IPointsOfInterestAnalyzer {
public:
    virtual ~IPointsOfInterestAnalyzer() = default;
    virtual std::vector<PointOfInterest> analyze(const cv::Mat& image) = 0;
};
