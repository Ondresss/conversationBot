#pragma once
#include <string>
#include <atomic>
class ConversationServerCache {
public:
    ConversationServerCache() = default;
    ~ConversationServerCache() = default;

    void setSpeechToTextOutput(const std::string& output) {
        this->speechToTextOutput = output;
    }
    const std::string& getSpeechToTextOutput() const {
        return speechToTextOutput;
    }
    void switchActiveState() {
        this->active = !this->active;
    }
    bool isActive() const {
        return active;
    }
private:
    std::string speechToTextOutput;
    std::atomic<bool> active = false;
};
