#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "../modules/core/PointOfInterest.h"

struct ImageAnalysis {
    std::vector<PointOfInterest> pointsOfInterest;
    std::vector<cv::Rect> facesBoxes;
};
