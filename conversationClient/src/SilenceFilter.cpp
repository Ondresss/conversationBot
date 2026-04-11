//
// Created by andrew on 01.03.26.
//

#include "../headers/SilenceFilter.h"


bool SilenceFilter::filter(const float* sample, unsigned int nBufferFrames) {
    double sum = 0.0;
    for (unsigned int i = 0; i < nBufferFrames; ++i) {
        sum += sample[i] * sample[i];
    }

    double rms = std::sqrt(sum / nBufferFrames);

    if (rms < threshold) {
        return true;
    }
    return false;
}
