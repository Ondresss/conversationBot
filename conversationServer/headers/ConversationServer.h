//
// Created by andrew on 07.03.26.
//

#pragma once
#include <memory>
#include "ServerInfo.h"
#include <vector>
#include "ServerSocket.h"
#include <thread>
#include <iostream>
#include "ServerHeader.h"
#include "SpeechToTextConverter.h"
#include "LLMGateway.h"
#include "TextToSpeechConverter.h"
#include <spdlog/spdlog.h>
class ConversationServer {
public:
    ConversationServer(ServerInfo serverInfo,
        const SpeechToTextConverter::ModelPath& modelPath,
        std::shared_ptr<LLMGateway> llmGateway_,
        const TextToSpeechConverter::ConfigParams& ttsParams_) {
        this->serverSocket = std::make_shared<ServerSocket>(std::move(serverInfo));
        this->speechToTextConverter = std::make_unique<SpeechToTextConverter>(modelPath);
        this->llmGateway = std::move(llmGateway_);
        this->textToSpeechConverter = std::make_unique<TextToSpeechConverter>(ttsParams_);
    };
    ~ConversationServer() {
        for (auto& th : this->clientThreads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void run();
    void handleClient(std::shared_ptr<ServerSocket::Client> client);
    std::vector<float> readAudioFromClient(const std::shared_ptr<ServerSocket::Client>& client);
    void writeResponse(const std::shared_ptr<ServerSocket::Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status);
private:
    std::shared_ptr<ServerSocket> serverSocket = nullptr;
    std::shared_ptr<LLMGateway> llmGateway = nullptr;
    std::vector<std::thread> clientThreads;
    std::unique_ptr<SpeechToTextConverter> speechToTextConverter = nullptr;
    std::unique_ptr<TextToSpeechConverter> textToSpeechConverter = nullptr;
};
