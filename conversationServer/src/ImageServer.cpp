#include "../headers/ImageServer.h"
#include "core/IAnalyzer.h"
#include "core/IFaceDetector.h"
#include "core/IPointsOfInterestAnalyzer.h"
#include "yuNetFaceModule/YuNetFaceDetector.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <spdlog/spdlog.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
ImageServer::ImageServer(ServerInfo serverInfo, ImageServerParams params, std::unordered_map<std::string, std::shared_ptr<IAnalyzer>> analyzers, std::shared_ptr<SharedContext> context) : AbstractServer(serverInfo, context), params(std::move(params)), analyzers(std::move(analyzers)) {}

std::shared_ptr<ImageServer> ImageServer::loadFromConfig(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        throw std::runtime_error("ImageServer::loadFromConfig(): Could not open file " + filename);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    nlohmann::json json = nlohmann::json::parse(ss.str());
    ImageServerParams params{};
    ServerInfo serverInfo{};
    std::unordered_map<std::string, std::shared_ptr<IAnalyzer>> analyzers;
    if(json.contains("imageServer")) {
        if(!json["imageServer"].contains("period")
            || !json["imageServer"].contains("noBufferedImages")
            || !json["imageServer"].contains("compressFormat") || !json["imageServer"].contains("ip")
            || !json["imageServer"].contains("port") || !json["imageServer"].contains("imageSpacingPeriod")) {
                throw std::runtime_error("ImageServer::loadFromConfig(): period, noBufferedImages, compressFormat, ip, port, and imageSpacingPeriod are required");
        }
        params.period = json["imageServer"].value("period", -1);
        params.noBufferedImages = json["imageServer"].value("noBufferedImages", -1);
        params.compressFormat = json["imageServer"].value("compressFormat", "none");
        serverInfo.ip = json["imageServer"].value("ip", "");
        serverInfo.port = json["imageServer"].value("port", -1);
        params.imageSpacingPeriod = json["imageServer"].value("imageSpacingPeriod", 1);

        if(!json["imageServer"].contains("analyzers")) {
            throw std::runtime_error("ImageServer::loadFromConfig(): missing analyzers field");
        }
        ImageServer::loadAnalyzers(analyzers, json);
        if(analyzers.empty()) {
            throw std::runtime_error("ImageServer::loadFromConfig(): No analyzers loaded");
        }
    } else {
        throw std::runtime_error("ImageServer::loadFromConfig(): No ImageServer section in config");
    }
    serverInfo.type = ServerType::Image;
    spdlog::info("ImageServer configuration loaded from '{}':\n"
                     "  -> IP: {}\n"
                     "  -> Port: {}\n"
                     "  -> Period: {}\n"
                     "  -> Image Spacing Period: {}\n"
                     "  -> Buffered Images: {}\n"
                     "  -> Compress Format: {}\n"
                     "  -> Loaded Analyzers Count: {}",
                     filename,
                     serverInfo.ip,
                     serverInfo.port,
                     params.period,
                     params.imageSpacingPeriod,
                     params.noBufferedImages,
                     params.compressFormat,
                     analyzers.size());
    return std::make_shared<ImageServer>(serverInfo, params, analyzers);
}
void ImageServer::loadAnalyzers(std::unordered_map<std::string, std::shared_ptr<IAnalyzer>>& analyzers, const nlohmann::json& config) {
    analyzers.clear();
    if (!config["imageServer"].contains("analyzers")) {
        throw std::runtime_error("ImageServer::loadAnalyzers(): No analyzers section in config");
    }
    for(const auto& analyzer : config["imageServer"]["analyzers"]) {
        const auto& type = analyzer["type"];
        std::string analyzerType = type.value("analyzerType", "unknown");
        std::string modelType = type.value("modelType", "unknown");

        if(analyzerType == "pointsOfInterest") {
            if(modelType == "yolo") {
                std::string classPath = analyzer.value("classPath", "");
                std::string modelPath = analyzer.value("modelPath", "");

                analyzers[analyzerType] = std::make_shared<YoloAnalyzer>(YoloAnalyzer::YoloParams{
                    .cocoClassesFile = classPath,
                    .netModelPath = modelPath
                });

                spdlog::info("Loaded YOLO Analyzer:\n"
                             "  -> Analyzer Type: {}\n"
                             "  -> Model Type: {}\n"
                             "  -> Model Path: {}\n"
                             "  -> Classes Path: {}",
                             analyzerType, modelType, modelPath, classPath);
            }
        }
        else if(analyzerType == "faceAnalyzer") {
            if(modelType == "yuNet") {
                std::string modelPath = analyzer["modelPath"].get<std::string>();
                int width = static_cast<int>(analyzer["inputWidth"]);
                int height = static_cast<int>(analyzer["inputHeight"]);

                float scoreThreshold = analyzer.value("scoreThreshold", 0.9f);
                float nmsThreshold = analyzer.value("nmsThreshold", 0.3f);

                analyzers[analyzerType] = std::make_shared<YuNetFaceDetector>(
                    modelPath,
                    cv::Size(width, height),
                    scoreThreshold,
                    nmsThreshold
                );

                spdlog::info("Loaded YuNet Face Detector:\n"
                             "  -> Analyzer Type: {}\n"
                             "  -> Model Type: {}\n"
                             "  -> Model Path: {}\n"
                             "  -> Input Resolution: {}x{}\n"
                             "  -> Score Threshold: {}\n"
                             "  -> NMS Threshold: {}",
                             analyzerType, modelType, modelPath, width, height, scoreThreshold, nmsThreshold);
            }
        } else if (analyzerType == "emotionAnalyzer") {
            if(modelType == "EfficientNet") {
                std::string modelPath = analyzer["modelPath"].get<std::string>();
                std::string classesPath = analyzer["classesPath"].get<std::string>();

                analyzers[analyzerType] = std::make_shared<EmotionAnalyzer>(
                    modelPath, classesPath
                );

                spdlog::info("Loaded Emotion Analyzer:\n"
                         "  -> Analyzer Type: {}\n"
                         "  -> Model Type: {}\n"
                         "  -> Model Path: {}",
                         analyzerType, modelType, modelPath);
            }
        }
    }
}

void ImageServer::sendDisconnectResponse(std::shared_ptr<Client> client) {
    ServerImageControlHeader header{};
    header.status = ServerImageStatus::DISCONNECT;
    const char* headerPtr = reinterpret_cast<const char*>(&header);
    ssize_t headerBytesLeft = sizeof(header);
    while (headerBytesLeft > 0) {
        ssize_t n = write(client->getDescriptors().videoFd, headerPtr, headerBytesLeft);
        if (n <= 0) {
            throw std::runtime_error("ImageServer::sendDisconnectResponse(): Error while writing header to client: "
                                        + std::string(strerror(errno)));
        }
        headerBytesLeft -= n;
        headerPtr += n;
    }
}


void ImageServer::disconnectAllClients() {
    const auto& clients = this->context->getClientRegistry();
    clients->forEachClient([this](const std::shared_ptr<Client>& client) {
        client->disconnect(ServerType::Image);
    });
    clients->clearRegistry();
    spdlog::info("ImageServer::disconnectAllClients(): All clients disconnected");
}

void ImageServer::run(std::stop_token stopToken) {
    try {
        if (!this->serverSocket) throw std::runtime_error("ImageServer::run(): serverSocket is null");
        std::stop_callback stopCb(stopToken, [this]() {
            if (this->serverSocket) {
                this->disconnectAllClients();
                this->serverSocket->shutdown();
            }
        });
        while (!stopToken.stop_requested()) {
            spdlog::info("ImageServer::run(): Waiting for new client....");
            auto client = this->serverSocket->waitForConnection();
            spdlog::info("ImageServer::run(): Client connected but is unauthenticated");
            if(client->getDescriptors().videoFd == -1) throw std::runtime_error("ImageServer::run(): Client videoFd is -1");
            this->authenticateClient(client);
            auto updatedClient = this->updateClientRegistry(client);
            spdlog::info("Image server: New client connected with IP {}",updatedClient->getIP());
            this->clientThreads.emplace_back(&ImageServer::handleClient, this, updatedClient);
        }
    } catch (std::exception& e){
        spdlog::error("ImageServer::run() -> ImageServer failed: " + std::string(e.what()));
    }
    spdlog::warn("Image Server was stopped");
}



void ImageServer::sendHeaderTCP(std::shared_ptr<Client> client, ServerImageControlHeader header) {
    ssize_t totalHeaderBytes = sizeof(header);
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerPtr = reinterpret_cast<char*>(&header);
    while (bytesLeft > 0) {
        ssize_t sent = send(client->getDescriptors().videoFd, headerPtr, bytesLeft, 0);
        if (sent == -1) throw std::runtime_error("ImageServer::sendHeaderTCP(): send() failed");
        bytesLeft -= sent;
        headerPtr += sent;
    }
}

void ImageServer::handleClient(std::shared_ptr<Client> client) {
    auto clientRegistry = this->context->getClientRegistry();
    auto imageServerContext = this->context->getImageServerContext();
    try {
        spdlog::info("Image server: Handling client with IP {}", client->getIP());
        std::vector<cv::Mat> bufferedFrames(this->params.noBufferedImages);
        ServerImageControlHeader header{.status = ServerImageStatus::OK, .periodMs = this->params.period, .imageCount = this->params.noBufferedImages, .compressType = "JPEG",.imageSpacingPeriod = this->params.imageSpacingPeriod};
        while (true) {
            client->getClientSync().acquireWorkerGate();
            ScopeLatchGuard latchGuard(*client);
            this->context->switchActiveStateForCache(ServerType::Image, true);
            spdlog::info("ImageServer: Acquired worker gate for client {}", client->getId());
            this->sendHeaderTCP(client, header);
            for (std::size_t i{0}; i < this->params.noBufferedImages; ++i) {
                this->recieveImageTCP(client, bufferedFrames[i]);
                spdlog::info("Image server: Received image {} for client {}", i, client->getId());
            }
            spdlog::info("Image server: total {} images recieved and buffered for client {}", this->params.noBufferedImages, client->getId());
            auto analysis = this->analyzeImages(bufferedFrames);
            spdlog::info("Analysis for client {}", client->getId());
            analysis->logAnalysis();
            imageServerContext->addCurrentAnalysis(analysis, client, bufferedFrames[0]);
            spdlog::debug("Image server: Finished latch count down for client {}", client->getId());
        }
    } catch (std::exception& e) {
        spdlog::error("ImageServer run: " + std::string(e.what()));
    }
    client->disconnect(ServerType::Image);
    clientRegistry->removeClient(client);
}


void ImageServer::recvHeaderTCP(std::shared_ptr<Client> client, ClientImageHeader& header) {
    ssize_t totalHeaderBytes = sizeof(header);
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerPtr = reinterpret_cast<char*>(&header);
    while (bytesLeft > 0) {
        ssize_t recvBytes = recv(client->getDescriptors().videoFd, headerPtr, bytesLeft, 0);
        if (recvBytes == -1) throw std::runtime_error("ImageServer::recvHeaderTCP(): recv() failed");
        bytesLeft -= recvBytes;
        headerPtr += recvBytes;
    }
    spdlog::debug("ImageServer::recvHeaderTCP: header received [totalBytes={}, width={}, height={}]", header.totalBytes, header.width, header.height);
}

void ImageServer::recieveImageTCP(std::shared_ptr<Client> client, cv::Mat& image) {
    ClientImageHeader header;
    this->recvHeaderTCP(client, header);
    ssize_t totalImageBytes = header.totalBytes;
    ssize_t bytesLeft = totalImageBytes;
    int8_t* imagePtr = new int8_t[totalImageBytes];
    int8_t* imagePtrStart = imagePtr;
    while (bytesLeft > 0) {
        ssize_t recvBytes = recv(client->getDescriptors().videoFd, imagePtr, bytesLeft, 0);
        if (recvBytes == -1) throw std::runtime_error("ImageServer::recieveImageTCP(): recv() failed");
        bytesLeft -= recvBytes;
        imagePtr += recvBytes;
    }
    image = cv::Mat(1, totalImageBytes, CV_8UC1, imagePtrStart).clone();
    delete[] imagePtrStart;
}

std::vector<PersonAnalysis> ImageServer::analyzePeople(std::vector<PointOfInterest> pointsOfInterest, const cv::Mat& image,const std::unordered_map<std::string, std::shared_ptr<IAnalyzer>>& analyzers) {
    std::vector<PersonAnalysis> results;
    auto itFaces = this->analyzers.find("faceAnalyzer");
    if (itFaces == this->analyzers.end()) {
        throw std::runtime_error("ImageServer::analyzeImages(): faceAnalyzer not found");
    }
    auto facesBoxesAnalyzer = std::dynamic_pointer_cast<IFaceDetector>(itFaces->second);
    if (!facesBoxesAnalyzer) {
        throw std::runtime_error("ImageServer::analyzeImages(): invalid type for faceAnalyzer");
    }
    auto itEmotion = this->analyzers.find("emotionAnalyzer");
    if (itEmotion == this->analyzers.end()) {
        throw std::runtime_error("ImageServer::analyzeImages(): emotionAnalyzer not found");
    }
    auto emotionAnalyzer = std::dynamic_pointer_cast<IEmotionAnalyzer>(itEmotion->second);
    if (!emotionAnalyzer) {
        throw std::runtime_error("ImageServer::analyzeImages(): invalid type for emotionAnalyzer");
    }
    for (const auto& point : pointsOfInterest) {
        if(point.name == "person") {
            PersonAnalysis person;
            cv::Mat cropped = image(point.boundingBox).clone();
            std::vector<cv::Rect> faces = facesBoxesAnalyzer->detectFaces(cropped);
            if (!faces.empty()) {
                person.faceBox = faces[0];
            } else {
                spdlog::warn("ImageServer::analyzeImages(): no faces detected for person at point {}", point.name);
                continue;
            }
            std::string emotion = emotionAnalyzer->analyze(cropped);
            if (!emotion.empty()) {
                person.emotion = emotion;
            } else {
                spdlog::warn("ImageServer::analyzeImages(): no emotions detected for person at point {}", point.name);
                continue;
            }
            person.coordinates = point.boundingBox;
            results.push_back(person);
        }
    }
    return results;
}

std::shared_ptr<ImageAnalysis> ImageServer::analyzeImages(const std::vector<cv::Mat>& images) {
    cv::Mat rawFrame = images.at(0);
    cv::Mat frame = cv::imdecode(rawFrame, cv::IMREAD_COLOR);
    if (frame.empty()) {
        throw std::runtime_error("ImageServer: Failed to decode image from TCP stream");
    }
    auto it = this->analyzers.find("pointsOfInterest");
    if (it == this->analyzers.end()) {
        throw std::runtime_error("ImageServer::analyzeImages(): pointsOfInterest analyzer not found");
    }
    auto pointsOfInterestAnalyzer = std::dynamic_pointer_cast<IPointsOfInterestAnalyzer>(it->second);
    if (!pointsOfInterestAnalyzer) {
        throw std::runtime_error("ImageServer::analyzeImages(): invalid type for pointsOfInterest analyzer");
    }

    std::vector<PointOfInterest> pointsOfInterest = pointsOfInterestAnalyzer->analyze(frame);
    std::vector<PersonAnalysis> people = this->analyzePeople(pointsOfInterest, frame, this->analyzers);
    return std::make_shared<ImageAnalysis>(pointsOfInterest, people);
}
