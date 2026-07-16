#pragma once
#include <cstdint>
#pragma pack(push, 1)
enum class ServerAuthStatus : uint32_t {
    Success = 0x0,
    Failure = 0x1,

};
struct ServerAuthResponseHeader {
    uint32_t status = 0x0;
};
#pragma pack(pop)
