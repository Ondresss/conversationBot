//
// Created by andrew on 05.03.26.
//

#pragma once
#include <memory>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>
#include "AbstractClient.h"
#include "AudioHandler.h"
#include "AudioPacket.h"
#include "ClientIdentifier.h"
#include "ServerAuthResponseHeader.h"
#include "ServerConversationHeader.h"
#include "ClientAuthHeader.h"
#include "ClientConversationHeader.h"

class ConversationClient : public AbstractClient {
public:
    explicit ConversationClient(ServerInfo serverInfo) : AbstractClient(serverInfo) {};
    void sendAudioPacket(const AudioPacket& audioPacket);
    std::tuple<const std::vector<std::int16_t>&,uint32_t> getResponseFromServer();
    void run() override;
private:
    std::vector<std::int16_t> responseBuffer;
    std::shared_ptr<AudioHandler> audioHandler = nullptr;
};
