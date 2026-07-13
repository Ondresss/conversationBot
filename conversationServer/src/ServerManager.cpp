#include "../headers/ServerManager.h"
void ServerManager::runAll() {
    for (auto& server : servers) {
        this->threads.emplace_back([server]() {
            server->run();
        });
    }
}

void ServerManager::setSharedContextAll(std::shared_ptr<SharedContext> context) {
    for (auto& server : servers) {
        server->setSharedContext(context);
    }
}

ServerManager::~ServerManager() {
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
