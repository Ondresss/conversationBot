#pragma once
#include <latch>
#include <semaphore>
#include <memory>
#include <optional>

class ClientSync {
public:
    ClientSync(int noWorkers = 1) : noWorkers(noWorkers) {
         this->finishedWorkLatch.emplace(noWorkers);
    };
    void updateFinishedWorkLatch() { this->finishedWorkLatch.emplace(this->noWorkers); }
    void waitForFinishedWork() { if (this->finishedWorkLatch) this->finishedWorkLatch->wait(); }
    void countDownFinishedWorkLatch() { if (this->finishedWorkLatch) this->finishedWorkLatch->count_down(); }
    void releaseWorkerGate() { this->workerGate.release(this->noWorkers); }
    void acquireWorkerGate() { this->workerGate.acquire(); }

    void setNoWorkers(int noWorkers) {
        this->noWorkers = noWorkers;
        this->updateFinishedWorkLatch();
    }
    int getNoWorkers() { return this->noWorkers; }
private:
    std::optional<std::latch> finishedWorkLatch;
    std::counting_semaphore<100> workerGate{0};
    int noWorkers;
};
