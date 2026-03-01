//
// Created by andrew on 01.03.26.
//

#include "../headers/SilenceFilter.h"


bool SilenceFilter::silence(float* sample, unsigned int nBufferFrames) {
    const float floatMax = std::numeric_limits<float>::max();
    const float floatMin = std::numeric_limits<float>::min();
    std::pair<int,int> minMaxIndexes{-1,-1};
    for (int i = 0; i < nBufferFrames; ++i) {

    }


}
