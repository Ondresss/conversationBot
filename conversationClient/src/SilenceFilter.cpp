//
// Created by andrew on 01.03.26.
//

#include "../headers/SilenceFilter.h"
#include  <iostream>

bool SilenceFilter::filter(const float* sample, unsigned int nBufferFrames) {
    double mean = 0.0;

    for (unsigned int i = 0; i < nBufferFrames; ++i) {
        mean += sample[i];
    }
    mean /= nBufferFrames;

    double sum = 0.0;
    for (unsigned int i = 0; i < nBufferFrames; ++i) {
        double centered = sample[i] - mean;
        sum += centered * centered;
    }

    double rms = std::sqrt(sum / nBufferFrames);

    if (rms < threshold) {
        return true;
    }
    return false;
}