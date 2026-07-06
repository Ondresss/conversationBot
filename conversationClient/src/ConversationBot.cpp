//
// Created by andrew on 01.03.26.
//
#include "../headers/ConversationBot.h"

#include <bits/this_thread_sleep.h>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/spdlog.h>

void ConversationBot::run() {
    if (!this->audioHandler) throw std::runtime_error("ConversationBot::run(): audioHandler is null");
    if (!this->client) throw std::runtime_error("ConversationBot::run(): client is null");
    this->client->connectToServer();
    this->audioHandler->startRecording();
    this->audioHandler->startPlayback();
    int noSilencePackets = 0;
    while (noSilencePackets < 1000) {
        if (!this->audioHandler->hasPackets()) continue;
        auto audioPacket = this->audioHandler->getNextAudioPacket();
        switch (audioPacket.type) {
        case AudioType::SILENCE:
            noSilencePackets++;
            spdlog::debug("Silence detected");
            if (!audioPacket.samples.empty()) this->client->sendAudioPacket(audioPacket);
            break;
        case AudioType::STARTOFSPEECH:
            noSilencePackets = 0;
            spdlog::debug("Start of speech detected\n");
            this->client->sendAudioPacket(audioPacket);
            break;
        case AudioType::ENDOFSPEECH: {
                noSilencePackets = 0;
                spdlog::debug("End of speech detected");
                this->client->sendAudioPacket(audioPacket);
                auto [response, status] = this->client->getResponseFromServer();
                if (status == 1) {
                    spdlog::warn("Sentence was too short");
                    continue;
                }
                if (status == 2) {
                    spdlog::info("Recieved empty response");
                    continue;
                }
                this->audioHandler->setPlaybackContextData(response);
                while (this->audioHandler->isSpeaking()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
                spdlog::info("Done talking");
                break;
        }
        case AudioType::SPEECH:
            noSilencePackets = 0;
            spdlog::debug("Speech detected");
            this->client->sendAudioPacket(audioPacket);
            break;
        default:
            spdlog::warn("Unrecognized speech pattern");
        }

    }
    this->audioHandler->stopRecording();
    this->client->disconnectFromServer();
}

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
