//
// Created by andrew on 4/18/26.
//

#include  "../headers/Client.h"

nlohmann::json Client::serialize() {
    nlohmann::json j;
    j["id"] = std::to_string(this->id);
    j["ip"] = this->clientIP;
    j["fd"] = this->clientFd;
    j["connected"] = this->isConnected;
    nlohmann::json conversation = nlohmann::json::array();

    for (const auto& entry : this->historyList) {
        conversation.push_back({
            {"question", entry.question},
            {"answer", entry.answer}
        });
    }

    j["history"] = conversation;

    return j;
}