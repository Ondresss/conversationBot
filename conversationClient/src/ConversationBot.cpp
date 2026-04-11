//
// Created by andrew on 01.03.26.
//
#include "../headers/ConversationBot.h"

#include <bits/this_thread_sleep.h>

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
            break;
        case AudioType::STARTOFSPEECH:
            noSilencePackets = 0;
            std::cout << "Start of speech detected\n";
            this->client->sendAudioPacket(audioPacket);
            break;
        case AudioType::ENDOFSPEECH: {
            noSilencePackets = 0;
            std::cout << "End of speech detected\n";
            this->client->sendAudioPacket(audioPacket);
            const auto& [response,status]  = this->client->getResponseFromServer();
            if (status == 1) {
                std::cout << "Sentence was too short\n";
                continue;
            }
            if (status == 2) {
                std::cout << "Empty response\n";
                continue;
            }
            this->audioHandler->setPlaybackContextData(response);
            while (this->audioHandler->isSpeaking()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Done talking\n";
            break;
        }
        case AudioType::SPEECH:
            noSilencePackets = 0;
            std::cout << "Speech detected\n";
            this->client->sendAudioPacket(audioPacket);
            break;
        default:
            std::cout << "Invalid audio packet type\n";
        }

    }
    this->audioHandler->stopRecording();
    this->client->disconnectFromServer();
}
