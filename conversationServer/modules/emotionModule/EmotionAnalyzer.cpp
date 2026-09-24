#include "EmotionAnalyzer.h"

void EmotionAnalyzer::loadClasses(const std::string& classesPath) {
    std::ifstream file(classesPath);
    if (!file.is_open()) {
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        this->emotions.push_back(line);
    }
}


std::string EmotionAnalyzer::analyze(const cv::Mat& frame) {
    if (frame.empty()) return "Unknown";

    cv::Mat rgbFace;
    cv::cvtColor(frame, rgbFace, cv::COLOR_BGR2RGB);

    cv::Mat blob = cv::dnn::blobFromImage(
        rgbFace,
        1.0 / 255.0,
        cv::Size(224, 224),
        cv::Scalar(0.485, 0.456, 0.406),
        true,
        false
    );

    net.setInput(blob);
    cv::Mat outputs = net.forward();

    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(outputs, 0, &confidence, 0, &classIdPoint);

    return emotions[classIdPoint.x];
}
