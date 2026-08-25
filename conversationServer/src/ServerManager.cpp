#include "../headers/ServerManager.h"
#include <memory>
#include <stop_token>

ServerManager::ServerManager() {}

ServerManager& ServerManager::getInstance() {
    static ServerManager manager;
    return manager;
}
void ServerManager::runAndWait() {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
    auto& manager = ServerManager::getInstance();
    manager.runAll();
    spdlog::info("Server is running: Waiting for signal...");
    int sig;
    sigwait(&sigset, &sig);
    spdlog::warn("Received signal {}, stopping all servers...", sig);
    manager.stopAll();
}
void ServerManager::runAll() {
    for (auto& server : servers) {
        this->threads.emplace_back([server](std::stop_token stopToken) {
            server->run(stopToken);
        });
    }
}

void ServerManager::stopAll() {
    for(auto& t : this->threads) {
        t.request_stop();
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
