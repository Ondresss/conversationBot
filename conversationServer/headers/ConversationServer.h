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
#include "../headers/ClientLogger.h"
#include <regex>
#include  "ConversationSession.h"
class ConversationServer {
public:
    struct WakeWordParams {
        bool useWakeWord = false;
        int sessionExpireTime = 0;
        std::string word;
    };
    ConversationServer(ServerInfo serverInfo,
        const SpeechToTextConverter::ModelPath& modelPath,
        std::shared_ptr<LLMGateway> llmGateway_,
        const TextToSpeechConverter::ConfigParams& ttsParams_,
        WakeWordParams wakeWordParams) {
        this->serverSocket = std::make_shared<ServerSocket>(std::move(serverInfo));
        this->speechToTextConverter = std::make_unique<SpeechToTextConverter>(modelPath);
        this->llmGateway = std::move(llmGateway_);
        this->textToSpeechConverter = std::make_unique<TextToSpeechConverter>(ttsParams_);

        if (wakeWordParams.useWakeWord) {
            this->conversationSession = std::make_unique<ConversationSession>(wakeWordParams.sessionExpireTime);
        }
        this->wordParams = wakeWordParams;

    };
    ~ConversationServer() {
        for (auto& th : this->clientThreads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void run();
    void handleClient(std::shared_ptr<Client> client);
    std::vector<float> readAudioFromClient(const std::shared_ptr<Client>& client,uint32_t& status);
    void writeResponse(const std::shared_ptr<Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status);

    bool handleSession(const std::string& response);

    [[nodiscard]] std::vector<std::shared_ptr<Client>>& getClients()  { return this->clients; }

    static std::shared_ptr<ConversationServer> loadFromConfig(const std::string& filename);
    void sendEmptyResponse(std::shared_ptr<Client> client,std::vector<float>& audioBuffer);
private:
    std::shared_ptr<ServerSocket> serverSocket = nullptr;
    std::shared_ptr<LLMGateway> llmGateway = nullptr;
    std::vector<std::thread> clientThreads;
    std::unique_ptr<SpeechToTextConverter> speechToTextConverter = nullptr;
    std::unique_ptr<TextToSpeechConverter> textToSpeechConverter = nullptr;
    std::unique_ptr<ConversationSession> conversationSession = nullptr;
    std::vector<std::shared_ptr<Client>> clients;
    std::mutex clientsMutex;

    WakeWordParams wordParams{};
};
