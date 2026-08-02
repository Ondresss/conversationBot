#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "../modules/core/PointOfInterest.h"
#include "Client.h"
class ImageServerCache {
public:
    struct AnalyzedImageData {
        std::vector<PointOfInterest> pointsOfInterest;
        cv::Mat processedImage;
        cv::Mat originalImage;
        std::string timestamp;
    };
    ImageServerCache() = default;
    ~ImageServerCache() = default;
    void addCurrentAnalysis(const std::vector<PointOfInterest>& data, std::shared_ptr<Client> client, const cv::Mat& inputImage);
    const std::shared_ptr<AnalyzedImageData>& getClientsAnalysis(std::size_t clientId) const;
private:
    std::shared_mutex cacheMutex;
    std::unordered_map<std::size_t,std::shared_ptr<AnalyzedImageData>> cache;

    cv::Mat processImage(const std::vector<PointOfInterest>& data,const cv::Mat& inputImage);
};
