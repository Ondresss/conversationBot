//
// Created by andrew on 5/24/26.
//

#include "../headers/ConversationSession.h"

bool ConversationSession::isSessionActive() {
    if (!this->isActive) return false;

    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastActivityTime);

    if (elapsed > sessionTimeout) {
        this->isActive = false;
        return false;
    }
    return true;
}

void ConversationSession::refreshSession() {
    this->isActive = true;
    this->lastActivityTime = std::chrono::steady_clock::now();
}