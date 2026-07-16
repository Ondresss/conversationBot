#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct ServerConversationHeader {
    uint32_t status = 0x0;
    uint32_t totalLen = 512;
};
#pragma pack(pop)
