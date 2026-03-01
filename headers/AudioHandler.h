//
// Created by andrew on 01.03.26.
//

#pragma once
#include <memory>
#include <rtaudio/RtAudio.h>
#include "IAudioFilter.h"

class AudioHandler {
public:
    AudioHandler(unsigned int noChannels,unsigned int firstChanel,unsigned int sampleRate, unsigned int bufferFrames);
    static int recordCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

    void startRecording();
    void stopRecording();
    void handleAudioInput(float* samples,unsigned int nBufferFrames);
private:
    void init();
    bool applyFilters(float* samples,unsigned int nBufferFrames);
    void loadDeviceIds();

    RtAudio::StreamParameters parameters;
    RtAudio audio;
    unsigned int sampleRate;
    unsigned int bufferFrames;
    std::vector<std::tuple<unsigned int,RtAudio::DeviceInfo>> devices;
    std::vector<std::shared_ptr<IAudioFilter>> filters;
};

