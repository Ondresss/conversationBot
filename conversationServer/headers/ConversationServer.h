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
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "../headers/ClientLogger.h"
#include <regex>
#include  "ConversationSession.h"
#include "ClientConversationHeader.h"
#include "AbstractServer.h"
class ConversationServer : public AbstractServer {
public:
    struct SessionParams {
        bool useWakeWord = false;
        int sessionExpireTime = 0;
        std::string word;
    };
    ConversationServer(ServerInfo serverInfo,
        const SpeechToTextConverter::ModelPath& modelPath,
        std::shared_ptr<LLMGateway> llmGateway_,
        const TextToSpeechConverter::ConfigParams& ttsParams_,
        SessionParams params,
        std::shared_ptr<SharedContext> context);

    void run() override;
    void handleClient(std::shared_ptr<Client> client) override;

    std::vector<float> readAudioFromClient(const std::shared_ptr<Client>& client,uint32_t& status);
    void writeResponse(const std::shared_ptr<Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status);

    bool handleSession(std::shared_ptr<Client> client,const std::string& response);

    static std::shared_ptr<ConversationServer> loadFromConfig(const std::string& filename);
    void sendEmptyResponse(std::shared_ptr<Client> client,std::vector<float>& audioBuffer);
private:
    std::shared_ptr<LLMGateway> llmGateway = nullptr;
    std::unique_ptr<SpeechToTextConverter> speechToTextConverter = nullptr;
    std::unique_ptr<TextToSpeechConverter> textToSpeechConverter = nullptr;
    SessionParams sessionParams{};
};
