#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#pragma pack(push, 1)

enum class ServerImageStatus : uint32_t {
    OK = 0,
    ERROR = 1,
    SEND = 2,
    INFO = 3,
    DISCONNECT = 4
};

struct ServerImageControlHeader {
    ServerImageStatus status = ServerImageStatus::ERROR;
    uint64_t periodMs = -1;
    uint32_t imageCount = -1;
    char compressType[16] = "JPEG";
    uint64_t imageSpacingPeriod = 1000;
};

#pragma pack(pop)

inline std::string toStringServerStatus(ServerImageStatus status) {
    switch (status) {
        case ServerImageStatus::OK: return "OK";
        case ServerImageStatus::ERROR: return "ERROR";
        case ServerImageStatus::SEND: return "SEND";
        case ServerImageStatus::INFO: return "INFO";
        default: return "UNKNOWN";
    }
}
