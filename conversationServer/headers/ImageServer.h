#include "ClientImageHeader.h"
#include "ServerSocket.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include "AbstractServer.h"
#include "SharedContext.h"
#include <nlohmann/json.hpp>
#include <opencv2/core/mat.hpp>
#include <spdlog/spdlog.h>
#include "ServerImageControlHeader.h"
#include <opencv2/opencv.hpp>
#include "../modules/core/IPointsOfInterestAnalyzer.h"
#include <fstream>
#include "../modules/yoloModule/YoloAnalyzer.h"
#include "core/PointOfInterest.h"

class ImageServer : public AbstractServer {
public:
    struct ImageServerParams {
        std::size_t period = 0;
        uint32_t noBufferedImages = 1;
        std::string compressFormat = "JPEG";
        std::size_t imageSpacingPeriod = 1;
    };
    ImageServer(ServerInfo serverInfo, ImageServerParams params, std::shared_ptr<IPointsOfInterestAnalyzer> pointsOfInterestAnalyzer, std::shared_ptr<SharedContext> context = nullptr);
    static std::shared_ptr<ImageServer> loadFromConfig(const std::string& filename);
    void run() override;
    void handleClient(std::shared_ptr<Client> client) override;

    void sendHeaderTCP(std::shared_ptr<Client> client, ServerImageControlHeader header);
    void recvHeaderTCP(std::shared_ptr<Client> client, ClientImageHeader& header);
    void recieveImageTCP(std::shared_ptr<Client> client, cv::Mat& image);
    void applyPointsOfInterestAnalysis(const std::vector<cv::Mat>& images);

private:
    ImageServerParams params{};
    std::shared_ptr<IPointsOfInterestAnalyzer> pointsOfInterestAnalyzer = nullptr;
    std::vector<PointOfInterest> currentPointsOfInterest;
};
