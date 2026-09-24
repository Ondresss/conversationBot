#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "../modules/core/PointOfInterest.h"
#include "Client.h"
#include "ImageAnalysis.h"
#include <atomic>
class ImageServerCache {
public:
    struct AnalyzedImageData {
        std::shared_ptr<ImageAnalysis> imageAnalysis = nullptr;
        cv::Mat processedImage;
        cv::Mat originalImage;
        std::string timestamp;

        nlohmann::json serialize() const {
            nlohmann::json jsonResponse;
            jsonResponse["pointsOfInterest"] = nlohmann::json::array();
            for (const auto& point : imageAnalysis->pointsOfInterest) {
                jsonResponse["pointsOfInterest"].push_back(point.serialize());
            }
            jsonResponse["people"] = nlohmann::json::array();
            for (const auto& person : imageAnalysis->people) {
                jsonResponse["people"].push_back(person.serialize());
            }
            jsonResponse["timestamp"] = timestamp;
            return jsonResponse;
        }
    };
    ImageServerCache() = default;
    ~ImageServerCache() = default;
    void addCurrentAnalysis(std::shared_ptr<ImageAnalysis> analysis, std::shared_ptr<Client> client, const cv::Mat& inputImage);
    const std::shared_ptr<AnalyzedImageData>& getClientsAnalysis(std::size_t clientId) const;
    void switchActiveState(bool state) { this->active = state; }
    bool isActive() const { return active; }
private:
    mutable std::shared_mutex cacheMutex;
    std::unordered_map<std::size_t,std::shared_ptr<AnalyzedImageData>> cache;
    std::atomic<bool> active = false;
    cv::Mat processImage(const std::vector<PointOfInterest>& data,const cv::Mat& inputImage);
};
