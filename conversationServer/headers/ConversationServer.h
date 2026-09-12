//
// Created by andrew on 07.03.26.
//

#pragma once
#include <cstdint>
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
#include "ServerStatus.h"
#include "LLMPrompt.h"
class ConversationServer : public AbstractServer {
public:
    enum class TriggerWordMechanism : uint32_t {
        NONE = 0,
        IGNORE = 1,
        WORD = 2
    };
    struct SessionParams {
        bool useWakeWord = false;
        int sessionExpireTime = 0;
        std::string word;
        TriggerWordMechanism triggerWordMechanism = TriggerWordMechanism::IGNORE;
        std::string triggerWord;
    };
    ConversationServer(ServerInfo serverInfo,
        const SpeechToTextConverter::ModelPath& modelPath,
        std::shared_ptr<LLMGateway> llmGateway_,
        const TextToSpeechConverter::ConfigParams& ttsParams_,
        SessionParams params,
        std::shared_ptr<SharedContext> context = nullptr);

    void run(std::stop_token stopToken) override;
    void handleClient(std::shared_ptr<Client> client) override;
    void disconnectAllClients() override;
    std::vector<float> readAudioFromClient(const std::shared_ptr<Client>& client,uint32_t& status);
    void writeResponse(const std::shared_ptr<Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status);
    bool handleSession(std::shared_ptr<Client> client,const std::string& response);
    static std::shared_ptr<ConversationServer> loadFromConfig(const std::string& filename);
    void sendEmptyResponse(std::shared_ptr<Client> client,std::vector<float>& audioBuffer);
    void sendDisconnectResponse(const std::shared_ptr<Client>& client);
private:
    std::shared_ptr<LLMGateway> llmGateway = nullptr;
    std::unique_ptr<SpeechToTextConverter> speechToTextConverter = nullptr;
    std::unique_ptr<TextToSpeechConverter> textToSpeechConverter = nullptr;
    SessionParams sessionParams{};

    void releaseWorkers(std::shared_ptr<Client>& client);
    bool containsTriggerWord(const std::string& text);
};
