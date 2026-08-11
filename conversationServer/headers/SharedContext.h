#pragma once
#include "ClientRegistry.h"
#include <latch>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <vector>
#include "ConversationServerCache.h"
#include "ImageServerCache.h"

class SharedContext {
public:
    SharedContext() : clientRegistry(std::make_shared<ClientRegistry>()), imageServerContext(std::make_shared<ImageServerCache>()), conversationServerContext(std::make_shared<ConversationServerCache>()) {
        spdlog::debug("Client Registry created in SharedContext");
        this->finishedWorkLatch.emplace(1);
    }
    const std::shared_ptr<ClientRegistry>& getClientRegistry() { return this->clientRegistry ? this->clientRegistry : throw std::runtime_error("ClientRegistry is not initialized"); }
    const std::shared_ptr<ImageServerCache>& getImageServerContext() { return this->imageServerContext ? this->imageServerContext : throw std::runtime_error("ImageServerContext is not initialized"); }
    const std::shared_ptr<ConversationServerCache>& getConversationServerContext() { return this->conversationServerContext ? this->conversationServerContext : throw std::runtime_error("ConversationServerContext is not initialized"); }
    void updateFinishedWorkLatch() { this->finishedWorkLatch.emplace(1); }
    void waitForFinishedWork() { if (this->finishedWorkLatch) this->finishedWorkLatch->wait(); }
    void countDownFinishedWorkLatch() { if (this->finishedWorkLatch) this->finishedWorkLatch->count_down(); }
    void releaseWorkerGate() { this->workerGate.release(1); }
    void acquireWorkerGate() { this->workerGate.acquire(); }

private:
    std::shared_ptr<ClientRegistry> clientRegistry = nullptr;
    std::shared_ptr<ImageServerCache> imageServerContext = nullptr;
    std::shared_ptr<ConversationServerCache> conversationServerContext = nullptr;
    std::optional<std::latch> finishedWorkLatch;
    std::counting_semaphore<100> workerGate{0};

};
