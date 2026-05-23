//
// Created by andrew on 07.03.26.
//

#include "../headers/ConversationServer.h"

#include "../headers/ClientHeader.h"



void ConversationServer::run() {
    try {
        if (!this->serverSocket) throw std::runtime_error("ConversationServer::run(): serverSocket is null");
        while (true) {
            spdlog::info("Waiting for new client....");
            auto client = this->serverSocket->waitForConnection();
            spdlog::info("New client joined with IP {}",client->getIP());
            this->clients.push_back(client);
            this->clientThreads.emplace_back(&ConversationServer::handleClient, this,client);
        }
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void ConversationServer::handleClient(std::shared_ptr<Client> client) {
    spdlog::info("Handling the client......");
    const SherpaOnnxOnlineStream* clientStream = nullptr;
    if (this->speechToTextConverter->isOnline()) clientStream = this->speechToTextConverter->createStream();

    std::vector<float> audioBuffer;
    try {
        while (true) {
            if (!client->getIsConnected()) {
                close(client->getFd());
                spdlog::warn("Client {} disconnected",client->getId());
                {
                    std::lock_guard<std::mutex> lock(this->clientsMutex);
                    auto it = std::find(this->clients.begin(), this->clients.end(), client);
                    if (it != this->clients.end()) {
                        this->clients.erase(it);
                    }
                }
                break;
            }
            uint32_t readStatus = 0x0;
            auto currentAudio = this->readAudioFromClient(client,readStatus);
            if (readStatus == 3) {
                spdlog::info("Read full sentence");
                if (audioBuffer.size() <= 12000) {
                    spdlog::error("Client sentence was too short!");
                    this->writeResponse(client,{},ServerStatus::TOOSHORT);
                    audioBuffer.clear();
                    continue;
                }
                std::string currentText = this->speechToTextConverter->processAudioChunk(clientStream, audioBuffer);
                spdlog::info("Audio processed");
                if (!currentText.empty()) {
                    spdlog::info("Input Text: {}", currentText);
                    std::string response = this->llmGateway->askLLM(currentText);
                    if (response == "IGNORE" || response == "IGNORE." || response.find("IGNORE") != std::string::npos) {
                        spdlog::info("LLM requested to IGNORE this sentence (detected foreign language or junk text).");
                        this->writeResponse(client, {},ServerStatus::EMPTY_RESPONSE);
                        continue;
                    }
                    auto logger = ClientLogger::getInstance();
                    logger.insertSpeech(*client,currentText,response);
                    spdlog::info("LLM RESPONSE: {}", response);
                    spdlog::debug("Starting Piper synthesis...");
                    try {
                        this->textToSpeechConverter->convertTextToSpeech(response);
                    } catch (const std::exception& e) {
                        spdlog::error("ERR: Exception during TTS");
                        spdlog::error("{}",e.what());
                        close(client->getFd());
                        client->setDisconnected();
                        spdlog::info("Closed connection with client");
                        return;
                    }
                    std::vector<int16_t> audioOut = this->textToSpeechConverter->getOutput();
                    spdlog::debug("Piper produced {} samples ({} bytes).", audioOut.size(), audioOut.size() * sizeof(int16_t));

                    this->writeResponse(client, audioOut,ServerStatus::OK);
                    spdlog::debug("Response written to socket.");
                } else {
                    spdlog::error("Empty text response!");
                    this->writeResponse(client,{},ServerStatus::EMPTY_RESPONSE);
                }
                audioBuffer.clear();
            } else {
                audioBuffer.insert(audioBuffer.end(), currentAudio.begin(), currentAudio.end());
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("void ConversationServer::handleClient(std::shared_ptr<ServerSocket::Client> client): {}",e.what());
    }
    this->speechToTextConverter->destroyStream(clientStream);
    client->setDisconnected();
    close(client->getFd());
    spdlog::debug("Stream destroyed");
}
std::vector<float> ConversationServer::readAudioFromClient(const std::shared_ptr<Client>& client,uint32_t& status) {
    std::vector<float> fullAudio;
    ClientHeader clientHeader{};
    char* headerPtr = reinterpret_cast<char*>(&clientHeader);
    ssize_t headerBytesLeft = sizeof(clientHeader);
    while (headerBytesLeft > 0) {
        ssize_t n = read(client->getFd(), headerPtr, headerBytesLeft);
        if (n == 0) throw std::runtime_error("ConversationServer::readAudioFromClient(): Client disconnected while sending header");
        if (n == -1) throw std::runtime_error("ConversationServer::readAudioFromClient(): Error while reading header from client " + std::string(strerror(errno)) );
        headerBytesLeft -= n;
        headerPtr += n;
    }
    size_t numSamples = clientHeader.packetLen / sizeof(float);
    status = clientHeader.status;
    if (client->getId() == 0x0) {
        client->setID(clientHeader.id);
        spdlog::info("Client identifier is: {}", client->getId());
        auto logger = ClientLogger::getInstance();
        logger.insert(*client);
        spdlog::info("Client inserted to DB!");
    }

    std::vector<float> packetData(numSamples);
    char* dataPtr = reinterpret_cast<char*>(packetData.data());
    ssize_t dataBytesLeft = clientHeader.packetLen;
    if (!numSamples || clientHeader.status == 3) {
        return {};
    }
    while (dataBytesLeft > 0) {
        ssize_t n = read(client->getFd(), dataPtr, dataBytesLeft);
        if (n <= 0) throw std::runtime_error("ConversationServer::readAudioFromClient(): Client disconnected or error while reading data from client");;
        dataBytesLeft -= n;
        dataPtr += n;
    }
    fullAudio.insert(fullAudio.end(), packetData.begin(), packetData.end());
    return fullAudio;
}

void ConversationServer::writeResponse(const std::shared_ptr<Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status) {
    spdlog::debug("Writing response back to client");
    ServerHeader header{};
    header.status = static_cast<uint32_t>(status);
    header.totalLen = soundBytes.size() * sizeof(std::int16_t);
    char* headerPtr = reinterpret_cast<char*>(&header);
    ssize_t headerBytesLeft = sizeof(header);
    while (headerBytesLeft > 0) {
        ssize_t n = write(client->getFd(), headerPtr, headerBytesLeft);
        if (n == 0) throw std::runtime_error("ConversationServer::writeResponse(): Client disconnected while sending header");
        if (n == -1) throw std::runtime_error("ConversationServer::writeResponse(): Error while reading header from client " + std::string(strerror(errno)) );
        headerBytesLeft -= n;
        headerPtr += n;
    }

    auto dataPtr = reinterpret_cast<const char*>(soundBytes.data());
    ssize_t dataBytesLeft = header.totalLen;
    while (dataBytesLeft > 0) {
        ssize_t n = write(client->getFd(), dataPtr, dataBytesLeft);
        if (n <= 0) throw std::runtime_error("ConversationServer::writeResponse(): Client disconnected while sending data");
        dataBytesLeft -= n;
        dataPtr += n;
    }
    spdlog::debug("Wrote response back to client");
}


std::shared_ptr<ConversationServer> ConversationServer::loadFromConfig(const std::string& filename) {

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        spdlog::error("ConversationServer::loadFromConfig(): Could not open file {}", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    nlohmann::json json = nlohmann::json::parse(ss.str());
    SpeechToTextConverter::ModelPath modelPath;
    LLMGateway::LLMParams params;
    TextToSpeechConverter::ConfigParams configParams;
    if (json.contains("STT") && json["STT"].contains("active_model")) {
        std::string activeModel = json["STT"]["active_model"];
        auto sttSection = json["STT"];

        if (activeModel == "senseVoice" && sttSection.contains("senseVoice")) {
            modelPath.encoderPath = sttSection["senseVoice"].value("model_path", "");
            modelPath.decoderPath = "";
            modelPath.tokenPath   = sttSection["senseVoice"].value("tokens_path", "");
            modelPath.joinerPath  = sttSection["senseVoice"].value("language", "en");
            modelPath.modelName   = "senseVoice";
        }
        else if (activeModel == "zipformer" && sttSection.contains("zipformer")) {
            modelPath.encoderPath = sttSection["zipformer"].value("encoder_path", "");
            modelPath.decoderPath = sttSection["zipformer"].value("decoder_path", "");
            modelPath.tokenPath   = sttSection["zipformer"].value("tokens_path", "");
            modelPath.joinerPath  = sttSection["zipformer"].value("joiner_path", "");
            modelPath.modelName   = "zipformer";
        }
        else if (activeModel == "whisper" && sttSection.contains("whisper")) {
            modelPath.encoderPath = sttSection["whisper"].value("encoder_path", "");
            modelPath.decoderPath = sttSection["whisper"].value("decoder_path", "");
            modelPath.tokenPath   = sttSection["whisper"].value("tokens_path", "");
            modelPath.joinerPath  = sttSection["whisper"].value("language", "cs");
            modelPath.modelName   = "whisper";
        }
        else {
            spdlog::error("ConversationServer::loadFromConfig(): Unknown or missing active STT model: {}", activeModel);
            std::exit(EXIT_FAILURE);
        }
    } else {
        spdlog::error("ConversationServer::loadFromConfig(): Missing 'STT' or 'active_model' in config");
        std::exit(EXIT_FAILURE);
    }
    if (json.contains("llm")) {
        auto llm = json["llm"];
        params.port = llm.value("port", 8080);
        params.binaryPath = llm.value("bin", "");
        params.modelPath = llm.value("instruct", "");
        params.language = llm.value("lang", "en");
    } else {
        spdlog::error("ConversationServer::loadFromConfig(): Missing 'llm' section in config");
        std::exit(EXIT_FAILURE);
    }
    if (json.contains("tts")) {
        if (json["tts"].value("lang","en") == "cs") {
            configParams.modelPath = json["tts"].value("model_path_cs", "");
            spdlog::info("Loaded Czech TTS model on path {}", configParams.modelPath);
        } else {
            configParams.modelPath = json["tts"].value("model_path_en", "");
            spdlog::info("Loaded English TTS model on path {}", configParams.modelPath);
        }
    } else {
        spdlog::warn("ConversationServer::loadFromConfig(): Missing 'tts' section, using defaults");
    }

    std::shared_ptr<LLMGateway> llmGateway = std::make_shared<LLMGateway>(params);
    ServerInfo serverInfo;

    if (json.contains("info")) {
        serverInfo.port = json["info"].value("port", 9999);
        serverInfo.ip = json["info"].value("ip", "0.0.0.0");
    } else {
        spdlog::error("ConversationServer::loadFromConfig(): Missing server info");
        std::exit(EXIT_FAILURE);
    }
    spdlog::info("Server attributes loaded successfuly");

    return std::make_shared<ConversationServer>(serverInfo,modelPath,llmGateway,configParams);

}
