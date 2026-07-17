#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#pragma pack(push, 1)

enum class ServerImageStatus{
    OK,
    ERROR,
    SEND,
    INFO
};

struct ServerImageControlHeader {
    ServerImageStatus status = ServerImageStatus::ERROR;
    uint64_t periodMs = -1;
    uint32_t imageCount = -1;
    char compressType[16] = "JPEG";
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
