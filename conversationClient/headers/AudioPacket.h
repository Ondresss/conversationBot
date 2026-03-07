//
// Created by andrew on 07.03.26.
//
#pragma once
#include "AudioType.h"
#include <vector>
struct AudioPacket {
    AudioType type;
    std::vector<float> samples;
};