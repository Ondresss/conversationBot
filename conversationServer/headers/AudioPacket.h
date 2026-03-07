//
// Created by andrew on 07.03.26.
//

#pragma once
#include <vector>
enum class AudioType {
    SPEECH = 1,
    SILENCE = 2,
    ENDOFSPEECH = 3,
    STARTOFSPEECH = 4,
    NONE = 0
};

struct AudioPacket {
    AudioType type;
    std::vector<float> samples;
};