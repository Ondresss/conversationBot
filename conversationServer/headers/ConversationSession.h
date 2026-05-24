//
// Created by andrew on 5/24/26.
//
#include <chrono>
#pragma once

class ConversationSession {
public:
    ConversationSession(int secs) : isActive(false),sessionTimeout(secs) {}
    bool isSessionActive();
    void refreshSession();

    void close() {
        this->isActive = false;
    }
private:
    bool isActive = false;
    std::chrono::steady_clock::time_point lastActivityTime;
    const std::chrono::seconds sessionTimeout{60};
};