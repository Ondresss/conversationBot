#pragma once
#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)

enum class ServerImageStatus{
    OK,
    ERROR,
    SEND,
    INFO
};

struct ServerImageControlHeader {
    ServerImageStatus status = ServerImageStatus::OK;
    uint64_t periodMs = 5000;
    uint32_t imageCount = 0;
    char compressType[16] = "JPEG";
    uint64_t imageSpacingPeriod = 1000;
};

#pragma pack(pop)
