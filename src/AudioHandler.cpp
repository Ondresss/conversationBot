//
// Created by andrew on 01.03.26.
//

#include "../headers/AudioHandler.h"

AudioHandler::AudioHandler(unsigned int noChannels,unsigned int firstChanel,unsigned int sampleRate, unsigned int bufferFrames) {
    this->loadDeviceIds();
    this->parameters = {
        this->audio.getDefaultInputDevice(),noChannels,firstChanel
    };
    this->bufferFrames = bufferFrames;
    this->sampleRate = sampleRate;

    this->init();
}

int AudioHandler::recordCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
    RtAudioStreamStatus status, void* userData) {

    if ( status ) std::cout << "Stream over/underflow detected." << std::endl;
    auto samples = static_cast<float*>(inputBuffer);

    std::cout << "nBufferFrames: " << nBufferFrames << std::endl;
    std::cout << "streamTime: " << streamTime << std::endl;
    std::cout << "status: " << status << std::endl;

    return 0;
}


void AudioHandler::stopRecording() {
    if (this->audio.isStreamRunning()) {
        this->audio.stopStream();
    }
}


void AudioHandler::startRecording() {
    try{
       this->audio.startStream();
    } catch( RtAudioError& e )
    {
        std::cerr << "Error when starting audio stream." << std::endl;
        std::cerr << e.getMessage() << std::endl;
        std::exit(EXIT_FAILURE);
    }

}

void AudioHandler::init() {
    try {
        this->audio.openStream(nullptr,&this->parameters,
    RTAUDIO_FLOAT32,this->sampleRate,&this->bufferFrames,&AudioHandler::recordCallback,this);
    } catch( RtAudioError& e ){
        std::cerr << "Error when initializing audio stream." << std::endl;
        std::cerr << e.getMessage() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void AudioHandler::handleAudioInput(float* samples, unsigned int nBufferFrames) {
    const unsigned int count = audio.getDeviceCount();
    for (unsigned int i = 0; i < count; i++) {
        RtAudio::DeviceInfo info = audio.getDeviceInfo(i);
        if (info.probed) {
            this->devices.emplace_back(i,info);
            std::cout << "Device " << i << ": " << info.name << std::endl;
        }
    }

}

void AudioHandler::loadDeviceIds()
{
}

