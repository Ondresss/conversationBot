#include "YoloAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <algorithm>


YoloAnalyzer::YoloAnalyzer(const YoloParams& params) : params(params) {
    this->loadCocoClasses(params.cocoClassesFile);
    this->load_net(this->net);
}

std::vector<PointOfInterest> YoloAnalyzer::analyze(const cv::Mat& frame) {
    cv::Mat processedFrame = this->preprocess(frame);
    cv::Mat blob;
    cv::dnn::blobFromImage(processedFrame, blob, 1./255., cv::Size(params.inputWidth, params.inputHeight), cv::Scalar(), true, false);
    net.setInput(blob);
    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    cv::Mat output = outputs.at(0);
    if (output.dims == 3) {
        output = output.reshape(1, output.size[1]);
    }
    cv::transpose(output, output);

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    float xFactor = static_cast<float>(processedFrame.cols) / params.inputWidth;
    float yFactor = static_cast<float>(processedFrame.rows) / params.inputHeight;

    int rows = output.rows;
    const int noClasses = this->cocoClasses.size();

    for (int i = 0; i < rows; ++i) {
        const float* rowData = output.ptr<float>(i);
        cv::Mat scores(1, noClasses, CV_32FC1, const_cast<float*>(rowData + 4));
        cv::Point classIdPoint;
        double maxClassScore;
        cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);
        float confidence = static_cast<float>(maxClassScore);
        if (confidence >= params.confidenceThreshold) {
            float cx = rowData[0];
            float cy = rowData[1];
            float w  = rowData[2];
            float h  = rowData[3];

            int left   = static_cast<int>((cx - 0.5f * w) * xFactor);
            int top    = static_cast<int>((cy - 0.5f * h) * yFactor);
            int width  = static_cast<int>(w * xFactor);
            int height = static_cast<int>(h * yFactor);

            boxes.push_back(cv::Rect(left, top, width, height));
            confidences.push_back(confidence);
            classIds.push_back(classIdPoint.x);
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, params.confidenceThreshold, params.nmsThreshold, indices);

    std::vector<PointOfInterest> results;
    results.reserve(indices.size());

    for (int idx : indices) {
        PointOfInterest poi;
        poi.name = this->cocoClasses.at(classIds.at(idx));
        poi.confidence = confidences.at(idx);
        poi.boundingBox = boxes.at(idx);

        results.push_back(poi);
    }

    return results;
}

void YoloAnalyzer::loadCocoClasses(const std::string& cocoClassesFile) {
    std::ifstream file(cocoClassesFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            this->cocoClasses.push_back(line);
        }
        file.close();
        spdlog::debug("YoloAnalyzer -> classes loaded");
    } else throw std::runtime_error("Failed to open coco classes file: " + cocoClassesFile);
}

void YoloAnalyzer::load_net(cv::dnn::Net &net) {
    auto result = cv::dnn::readNet(params.netModelPath);
    net = result;
    spdlog::debug("YoloAnalyzer -> opencv net loaded");
}

cv::Mat YoloAnalyzer::preprocess(const cv::Mat& frame) {
    if (frame.empty()) {
            throw std::runtime_error("Input frame for YoloAnalyzer is empty!");
    }

    cv::Mat input;

    if (frame.rows == 1 || frame.cols == 1) {
        input = cv::imdecode(frame, cv::IMREAD_COLOR);
        if (input.empty()) {
            throw std::runtime_error("Failed to decode JPEG image buffer!");
        }
    } else {
        input = frame;
    }

    if (input.channels() == 1) {
        cv::cvtColor(input, input, cv::COLOR_GRAY2BGR);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, input, cv::COLOR_BGRA2BGR);
    }
    int col = input.cols;
    int row = input.rows;
    int _max = std::max(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    input.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}
