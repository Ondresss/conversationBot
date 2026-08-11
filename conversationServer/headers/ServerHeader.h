//
// Created by andrew on 07.03.26.
//

#pragma once
#include <cstdint>
#include "ServerStatus.h"
#pragma pack(push, 1)
struct ServerHeader {
    uint32_t status = static_cast<uint32_t>(ServerStatus::OK);
    uint32_t totalLen = 512;
};
#pragma pack(pop)
