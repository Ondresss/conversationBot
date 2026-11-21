#pragma once
#include <atomic>
class IWorker {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void fetchLoop() = 0;
    virtual ~IWorker() = default;
protected:
    std::atomic<bool> running{false};
};