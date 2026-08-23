#pragma once
#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)

enum class ServerImageStatus : uint32_t{
    OK = 0,
    ERROR = 1,
    SEND = 2,
    INFO = 3,
    DISCONNECT = 4
};

struct ServerImageControlHeader {
    ServerImageStatus status = ServerImageStatus::OK;
    uint64_t periodMs = 5000;
    uint32_t imageCount = 0;
    char compressType[16] = "JPEG";
    uint64_t imageSpacingPeriod = 1000;
};

#pragma pack(pop)
