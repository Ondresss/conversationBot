//
// Created by andrew on 01.03.26.
//

#pragma once
#include <limits>
#include <utility>
class SilenceFilter {
public:
    explicit SilenceFilter(float threshHold) : threshold(threshHold) {};
    bool silence(float* sample,unsigned int nBufferFrames);
    void setThreshHold(float threshHold) { this->threshold = threshHold; }
private:
    float threshold;
};