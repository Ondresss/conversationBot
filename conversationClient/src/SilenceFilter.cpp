//
// Created by andrew on 01.03.26.
//

#include "../headers/SilenceFilter.h"


bool SilenceFilter::filter(const float* sample, unsigned int nBufferFrames) {
    float sampleMin = std::numeric_limits<float>::max();
    float sampleMax = std::numeric_limits<float>::min();
    for (int i = 0; i < nBufferFrames; ++i) {
        if (sample[i] < sampleMin){
            sampleMin = sample[i];
        }
        if (sample[i] > sampleMax) {
            sampleMax = sample[i];
        }
    }
    float diff = sampleMax - sampleMin;
    if (diff < threshold){
        return true;
    }
    return false;
}
