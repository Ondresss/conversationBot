#pragma once
#include "core/IAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <vector>
class IFaceDetector : public IAnalyzer {
public:
  virtual std::vector<cv::Rect> detectFaces(const cv::Mat& frame) = 0;
};
