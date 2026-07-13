#include "ClientImageHeader.h"
#include "ServerSocket.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include "AbstractServer.h"
#include "SharedContext.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "ServerImageControlHeader.h"
#include <opencv2/opencv.hpp>
#include <fstream>
class ImageServer : public AbstractServer {
public:
    struct ImageServerParams {
        std::size_t period = 0;
        uint32_t noBufferedImages = 1;
        std::string compressFormat = "JPEG";
    };
    ImageServer(ServerInfo serverInfo, ImageServerParams params, std::shared_ptr<SharedContext> context);
    static std::shared_ptr<ImageServer> loadFromConfig(const std::string& filename);
    void run() override;
    void handleClient(std::shared_ptr<Client> client) override;

    void sendHeaderTCP(std::shared_ptr<Client> client, ServerImageControlHeader header);
    void recvHeaderTCP(std::shared_ptr<Client> client, ClientImageHeader& header);
    void recieveImageTCP(std::shared_ptr<Client> client, cv::Mat& image);

private:
    ImageServerParams params{};
};
