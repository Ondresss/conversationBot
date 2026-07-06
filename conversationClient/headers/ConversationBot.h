//
// Created by andrew on 01.03.26.
//
#pragma once
#include <memory>
#include "AudioHandler.h"
#include "ConversationClient.h"
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
class ConversationBot{
public:
    ConversationBot(std::shared_ptr<AudioHandler> audioHandler,std::shared_ptr<ConversationClient> client)
    : audioHandler(std::move(audioHandler)),client(std::move(client)) {}
    void run();
    static void initLogging();
private:
    std::shared_ptr<AudioHandler> audioHandler;
    std::shared_ptr<ConversationClient> client;
};
