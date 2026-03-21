//
// Created by andrew on 01.03.26.
//
#include "../headers/ConversationBot.h"

void ConversationBot::run() {
    if (!this->audioHandler) throw std::runtime_error("ConversationBot::run(): audioHandler is null");
    if (!this->client) throw std::runtime_error("ConversationBot::run(): client is null");
    this->client->connectToServer();
    this->audioHandler->startRecording();
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
        case AudioType::ENDOFSPEECH:
            noSilencePackets = 0;
            std::cout << "End of speech detected\n";
            this->client->sendAudioPacket(audioPacket);
            break;
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
