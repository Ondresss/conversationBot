//
// Created by andrew on 07.03.26.
//

#pragma once
#include <cstdint>
enum class ServerStatus : uint32_t {
    OK = 0,
    TOOSHORT = 1,
    EMPTY_RESPONSE = 2,
};

#pragma pack(push, 1)
struct ServerHeader {
    uint32_t status = static_cast<uint32_t>(ServerStatus::OK);
    uint32_t totalLen = 512;
};
#pragma pack(pop)