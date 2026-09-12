#include "../headers/ImageServerCache.h"
#include <mutex>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <shared_mutex>
#include <spdlog/spdlog.h>

void ImageServerCache::addCurrentAnalysis(std::shared_ptr<ImageAnalysis> analysis, std::shared_ptr<Client> client, const cv::Mat& inputImage) {
    cv::Mat decodedImage = cv::imdecode(inputImage, cv::IMREAD_COLOR);
    if (decodedImage.empty()) {
        throw std::runtime_error("ImageServerCache: Failed to decode JPEG image from client " + std::to_string(client->getId()));
    }
    cv::Mat processed = this->processImage(analysis->pointsOfInterest, decodedImage);
    std::unique_lock<std::shared_mutex> lock(this->cacheMutex);
    auto it = this->cache.find(client->getId());
    if(it == this->cache.end()) {
        this->cache[client->getId()] = std::make_shared<AnalyzedImageData>(AnalyzedImageData{.pointsOfInterest = analysis->pointsOfInterest, .processedImage = this->processImage(analysis->pointsOfInterest, decodedImage), .originalImage = inputImage.clone(), .timestamp = client->getUptime()});
        return;
    }
    auto analyzedData = this->cache[client->getId()];
    analyzedData->pointsOfInterest = analysis->pointsOfInterest;
    analyzedData->processedImage = this->processImage(analysis->pointsOfInterest, decodedImage);
    analyzedData->originalImage = inputImage.clone();
    analyzedData->timestamp = client->getUptime();
}


cv::Mat ImageServerCache::processImage(const std::vector<PointOfInterest>& data, const cv::Mat& inputImage) {
    cv::Mat outputImage = inputImage.clone();
    for (const auto& poi : data) {
        cv::rectangle(outputImage, poi.boundingBox, cv::Scalar(0, 255, 0), 2);
        cv::putText(outputImage, poi.name, cv::Point(poi.boundingBox.x, poi.boundingBox.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 255, 0), 2);
    }
    cv::imwrite("output.jpg", outputImage);
    return outputImage;
}


const std::shared_ptr<ImageServerCache::AnalyzedImageData>& ImageServerCache::getClientsAnalysis(std::size_t clientId) const {
    std::shared_lock<std::shared_mutex> lock(this->cacheMutex);
    return this->cache.at(clientId);
}
