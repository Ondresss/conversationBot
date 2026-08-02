#include "../headers/ImageServerCache.h"
#include <mutex>
#include <shared_mutex>

void ImageServerCache::addCurrentAnalysis(const std::vector<PointOfInterest>& data, std::shared_ptr<Client> client, const cv::Mat& inputImage) {
   std::unique_lock<std::shared_mutex> lock(this->cacheMutex);
   auto it = this->cache.find(client->getId());
   if(it == this->cache.end()) {
      this->cache[client->getId()] = std::make_shared<AnalyzedImageData>(AnalyzedImageData{.pointsOfInterest = data, .processedImage = processImage(data, inputImage), .originalImage = inputImage.clone(), .timestamp = client->getUptime()});
   }
   auto analyzedData = this->cache[client->getId()];
   analyzedData->pointsOfInterest = data;
   analyzedData->processedImage = this->processImage(data, inputImage);
   analyzedData->originalImage = inputImage.clone();
   analyzedData->timestamp = client->getUptime();

}


cv::Mat ImageServerCache::processImage(const std::vector<PointOfInterest>& data, const cv::Mat& inputImage) {
    cv::Mat outputImage = inputImage.clone();
    for (const auto& poi : data) {
        cv::rectangle(outputImage, poi.boundingBox, cv::Scalar(0, 255, 0), 2);
        cv::putText(outputImage, poi.name, cv::Point(poi.boundingBox.x, poi.boundingBox.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 255, 0), 2);
    }
    return outputImage;
}


const std::shared_ptr<ImageServerCache::AnalyzedImageData>& ImageServerCache::getClientsAnalysis(std::size_t clientId) const {
    return this->cache.at(clientId);
}
