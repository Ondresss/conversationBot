#pragma once
#include "ServerStatus.h"
#include <cstdint>
#pragma pack(push, 1)
struct ServerConversationHeader {
    ServerStatus status = ServerStatus::OK;
    uint32_t totalLen = 512;
};
#pragma pack(pop)
