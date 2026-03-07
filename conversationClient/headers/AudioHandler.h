//
// Created by andrew on 01.03.26.
//

#pragma once
#include <memory>
#include <queue>
#include <rtaudio/RtAudio.h>
#include "IAudioFilter.h"
#include "AudioType.h"
#include "AudioPacket.h"
class AudioHandler {
public:
    AudioHandler(unsigned int noChannels,unsigned int firstChanel,unsigned int sampleRate, unsigned int bufferFrames);
    static int recordCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

    void startRecording();
    void stopRecording();

    AudioPacket getNextAudioPacket();
    void pushAudioPacket(AudioPacket audioPacket) { this->audioQueue.push(std::move(audioPacket)); }
    std::mutex& getMutex() {return this->queueMtx;}
    int checkForEndSentence(AudioType type);
    int checkForStartSentence();
    void resetSilenceCounter() {this->silenceCounter = 0;};
    void setLastAudioType(AudioType type) {this->lastAudioType = type;}
    [[nodiscard]] AudioType getLastAudioType() const {return this->lastAudioType;}
    bool hasPackets();

private:
    void init();
    AudioType applyFilters(const float* samples,unsigned int nBufferFrames);
    void loadDeviceIds();

    RtAudio::StreamParameters parameters;
    RtAudio audio;
    unsigned int sampleRate;
    unsigned int bufferFrames;
    std::vector<std::tuple<unsigned int,RtAudio::DeviceInfo>> devices;
    std::vector<std::shared_ptr<IAudioFilter>> filters;
    std::mutex queueMtx;
    std::queue<AudioPacket> audioQueue;

    int silenceCounter = 0;
    int speechCounter = 0;
    AudioType lastAudioType = AudioType::NONE;
};

