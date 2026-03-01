//
// Created by andrew on 01.03.26.
//
#pragma once
#include <memory>
#include "AudioHandler.h"
class ConversationBot {
public:
    ConversationBot(std::shared_ptr<AudioHandler> audioHandler,std::string modelPath)
    : audioHandler(audioHandler),modelPath(std::move(modelPath)) {}

private:
    std::shared_ptr<AudioHandler> audioHandler;
    std::string modelPath;
};
