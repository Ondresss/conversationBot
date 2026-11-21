#pragma once
#include "Client.h"

struct ScopeLatchGuard {
    Client& client;
    bool dismissed = false;
    explicit ScopeLatchGuard(Client& c) : client(c) {}
    ~ScopeLatchGuard() {
        if (!dismissed) {
            client.getClientSync().countDownFinishedWorkLatch();
        }
    }
    void dismiss() { dismissed = true; }
};