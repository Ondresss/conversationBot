//
// Created by andrew on 01.03.26.
//
#pragma once
#include "AudioType.h"
class IAudioFilter {
public:
    IAudioFilter() = default;
    virtual ~IAudioFilter() = default;
    virtual bool filter(const float* sample,unsigned int nBufferFrames) = 0;
    [[nodiscard]] virtual AudioType getActionType() const = 0;
};