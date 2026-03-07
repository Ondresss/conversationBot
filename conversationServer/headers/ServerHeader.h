//
// Created by andrew on 07.03.26.
//

#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct ServerHeader {
    uint32_t status = 0x0;
    uint32_t totalLen = 512;
};
#pragma pack(pop)