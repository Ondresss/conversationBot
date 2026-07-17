//
// Created by andrew on 01.03.26.
//
#include "../headers/ConversationBot.h"

#include <bits/this_thread_sleep.h>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/spdlog.h>

void ConversationBot::initLogging() {
    spdlog::init_thread_pool(8192,1);
    auto async_file_logger = spdlog::basic_logger_mt<spdlog::async_factory>(
                "main_logger",
                "../log.txt"
            );
    async_file_logger->flush_on(spdlog::level::debug);
    async_file_logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(async_file_logger);
}

void ConversationBot::run() {
    for(auto& client : clients) {
        serverThreads.emplace_back([&client]() {
            client->run();
        });
    }
    for(auto& thread : serverThreads) {
        thread.join();
    }
}

ConversationBot::~ConversationBot() {
    for(auto& thread : serverThreads) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}
