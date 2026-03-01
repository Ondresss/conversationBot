//
// Created by andrew on 01.03.26.
//

#pragma once
#include <limits>
#include <utility>
#include "IAudioFilter.h"
class SilenceFilter final : public IAudioFilter {
public:
    explicit SilenceFilter(float threshHold) : threshold(threshHold) {};
    bool filter(const float* sample,unsigned int nBufferFrames) override;
    void setThreshHold(float threshHold) { this->threshold = threshHold; }
private:
    float threshold;
};