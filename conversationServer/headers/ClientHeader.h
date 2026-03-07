//
// Created by andrew on 07.03.26.
#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct ClientHeader {
    uint32_t status = 0x0;
    uint32_t packetLen = 512;
};
#pragma pack(pop)