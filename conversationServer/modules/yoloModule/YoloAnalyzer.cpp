module;
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include "../core/IPointsOfInterestAnalyzer.h"
#include <fstream>
export module yoloAnalyzer;

export class YoloAnalyzer : public IPointsOfInterestAnalyzer {
public:
    struct YoloParams {
        float inputWidth = 640.0;
        float inputHeight = 640.0;
        float scoreThreshold = 0.5;
        float nmsThreshold = 0.5;
        float confidenceThreshold = 0.5;
        std::string cocoClassesFile;
        std::string netModelPath;
    };
    YoloAnalyzer(const YoloParams& params) : params(params) {
        this->loadCocoClasses(params.cocoClassesFile);
        this->load_net(this->net);
    }
    std::vector<PointOfInterest> analyze(const cv::Mat& frame) override {
        cv::Mat processedFrame = this->preprocess(frame);
        cv::Mat blob;
        cv::dnn::blobFromImage(processedFrame, blob, 1./255., cv::Size(params.inputWidth, params.inputHeight), cv::Scalar(), true, false);
        net.setInput(blob);
        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        float xFactor = static_cast<float>(processedFrame.cols) / params.inputWidth;
        float yFactor = static_cast<float>(processedFrame.rows) / params.inputHeight;

        float* data = reinterpret_cast<float*>(outputs[0].data);

        int rows = outputs[0].size[1];
        int dimensions = outputs[0].size[2];

        for (int i = 0; i < rows; ++i) {
            float objectness = data[4];

            if (objectness >= this->params.scoreThreshold) {
                float* classesScores = data + 5;
                cv::Mat scores(1, params.nmsThreshold, CV_32FC1, classesScores);

                cv::Point classIdPoint;
                double maxClassScore;
                cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);

                float totalConfidence = objectness * static_cast<float>(maxClassScore);

                if (totalConfidence >= params.confidenceThreshold) {
                    float cx = data[0];
                    float cy = data[1];
                    float w = data[2];
                    float h = data[3];

                    int left = static_cast<int>((cx - 0.5f * w) * xFactor);
                    int top = static_cast<int>((cy - 0.5f * h) * yFactor);
                    int width = static_cast<int>(w * xFactor);
                    int height = static_cast<int>(h * yFactor);

                    boxes.push_back(cv::Rect(left, top, width, height));
                    confidences.push_back(totalConfidence);
                    classIds.push_back(classIdPoint.x);
                }
            }
            data += dimensions;
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
private:
    cv::dnn::Net net;
    std::vector<std::string> cocoClasses;
    YoloParams params;

    void loadCocoClasses(const std::string& cocoClassesFile) {
        std::ifstream file(cocoClassesFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                this->cocoClasses.push_back(line);
            }
            file.close();
        } else throw std::runtime_error("Failed to open coco classes file: " + cocoClassesFile);
    }

    void load_net(cv::dnn::Net &net){
        auto result = cv::dnn::readNet(params.netModelPath);
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        net = result;
    }
    cv::Mat preprocess(const cv::Mat& frame) {
        int col = frame.cols;
        int row = frame.rows;
        int _max = std::max(col, row);
        cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
        frame.copyTo(result(cv::Rect(0, 0, col, row)));
        return result;
    }


};
