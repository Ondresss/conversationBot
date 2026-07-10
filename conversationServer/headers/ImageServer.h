#include "ServerSocket.h"
#include <cstddef>
#include <memory>
#include "AbstractServer.h"
#include "SharedContext.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

class ImageServer : public AbstractServer {
public:
    struct ImageServerParams {
        std::size_t period = 0;
        int noBufferedImages = 1;
        std::string compressFormat = "JPEG";
    };
    ImageServer(ServerInfo serverInfo, ImageServerParams params, std::shared_ptr<SharedContext> context);
    static std::shared_ptr<ImageServer> loadFromConfig(const std::string& filename);
    void run() override;
private:
    std::unique_ptr<ServerSocket> serverSocket = nullptr;
    ImageServerParams params{};
};
