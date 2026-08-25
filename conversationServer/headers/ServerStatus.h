#pragma once
#pragma pack(push, 1)
enum class ServerStatus {
    OK = 0,
    TOO_SHORT = 1,
    EMPTY_RESPONSE = 2,
    PARTIAL_RESPONSE = 3,
    DISCONNECT = 4,
};
#pragma pack(pop)
