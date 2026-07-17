//
// Created by andrew on 01.03.26.
//
#pragma once
#include <memory>
#include "AbstractClient.h"
#include "AudioHandler.h"
#include "ConversationClient.h"
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
class ConversationBot{
public:
    ConversationBot(const std::vector<std::shared_ptr<AbstractClient>>& clients)
    : clients(clients) {}
    ~ConversationBot();
    void run();
    static void initLogging();
private:
    std::vector<std::shared_ptr<AbstractClient>> clients;
    std::vector<std::jthread> serverThreads;
};
