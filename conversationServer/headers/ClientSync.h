#pragma once
#include <latch>
#include <semaphore>
#include <memory>
#include <optional>

class ClientSync {
public:
    ClientSync()  {
         this->finishedWorkLatch.emplace(1);
    };
    void updateFinishedWorkLatch() { this->finishedWorkLatch.emplace(1); }
    void waitForFinishedWork() { if (this->finishedWorkLatch) this->finishedWorkLatch->wait(); }
    void countDownFinishedWorkLatch() { if (this->finishedWorkLatch) this->finishedWorkLatch->count_down(); }
    void releaseWorkerGate() { this->workerGate.release(1); }
    void acquireWorkerGate() { this->workerGate.acquire(); }
private:
    std::optional<std::latch> finishedWorkLatch;
    std::counting_semaphore<100> workerGate{0};
};
