#pragma once
#include "../core/IFaceDetector.h"
#include <opencv2/core/types.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <vector>
class YuNetFaceDetector : public IFaceDetector {
public:
    YuNetFaceDetector(const std::string& modelPath, const cv::Size& inputSize, float scoreThreshold = 0.8f, float nmsThreshold = 0.3f);
    void setInputSize(const cv::Size& inputSize) {
        this->detector->setInputSize(inputSize);
    }
    std::vector<cv::Rect> detectFaces(const cv::Mat& frame) override;
private:
    cv::Ptr<cv::FaceDetectorYN> detector;
    cv::Size inputSize;
};
