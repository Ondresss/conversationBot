#include "YuNetFaceDetector.h"

YuNetFaceDetector::YuNetFaceDetector(const std::string& modelPath, const cv::Size& inputSize, float scoreThreshold, float nmsThreshold) {
    this->detector = cv::FaceDetectorYN::create(
        modelPath,
        "",
        inputSize,
        scoreThreshold,
        nmsThreshold
    );
}
std::vector<cv::Rect> YuNetFaceDetector::detectFaces(const cv::Mat& frame) {
    std::vector<cv::Rect> faceBoxes;

    if (frame.empty()) return faceBoxes;

    float scaleX = static_cast<float>(frame.cols) / this->inputSize.width;
    float scaleY = static_cast<float>(frame.rows) / this->inputSize.height;

    cv::Mat resizedFrame;
    cv::resize(frame, resizedFrame, this->inputSize);

    cv::Mat faces;
    this->detector->detect(resizedFrame, faces);

    for (int i = 0; i < faces.rows; ++i) {
        float x = faces.at<float>(i, 0) * scaleX;
        float y = faces.at<float>(i, 1) * scaleY;
        float w = faces.at<float>(i, 2) * scaleX;
        float h = faces.at<float>(i, 3) * scaleY;

        cv::Rect faceRect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h));
        faceRect = faceRect & cv::Rect(0, 0, frame.cols, frame.rows);

        if (faceRect.width > 0 && faceRect.height > 0) {
            faceBoxes.push_back(faceRect);
        }
    }

    return faceBoxes;
}
