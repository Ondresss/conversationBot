//
// Created by andrew on 01.03.26.
//

#include "../headers/AudioHandler.h"

#include "../headers/SilenceFilter.h"
#include <cstdlib>
#include <iostream>
#include <spdlog/spdlog.h>

#define SHOW_ERROR(text)  spdlog::error(text); \
                            std::cerr << text ;

AudioHandler::AudioHandler(unsigned int noChannels,unsigned int firstChanel,unsigned int sampleRate, unsigned int bufferFrames, double noiseThreshold,int endSentenceThreshold) : endSentenceThreshold(endSentenceThreshold),
        bufferFrames(bufferFrames),
        sampleRate(sampleRate),
        noiseThreshold(noiseThreshold) {

    this->loadDeviceIds();

    unsigned int defaultId = this->audio.getDefaultInputDevice();

    if (defaultId == 0 && this->audio.getDeviceCount() == 0) {
        throw std::runtime_error("Could not find sound devices");
    }

    spdlog::info("Default input devide id {}",defaultId);
    this->parameters = {.deviceId = defaultId,.nChannels = noChannels,.firstChannel = firstChanel};
    this->init();
}

int AudioHandler::checkForEndSentence(AudioType type) {
    if (type == AudioType::SILENCE) {
        this->silenceCounter++;
    } else {
        this->silenceCounter = 0;
        return 0;
    }

    if (this->silenceCounter >= this->endSentenceThreshold && this->speechCounter > 0) {
        this->speechCounter = 0;
        this->silenceCounter = 0;
        return 1;
    }
    return 0;
}

int AudioHandler::checkForStartSentence() {
    if (this->speechCounter > 0) return 0;

    this->speechCounter = 1;
    return 1;
}


bool AudioHandler::hasPackets() {
    std::lock_guard<std::mutex> lock(this->queueMtx);
    return !this->audioQueue.empty();
}


int AudioHandler::recordCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
    RtAudioStreamStatus status, void* userData) {
    auto handler = static_cast<AudioHandler*>(userData);
    if ( status ) {
        spdlog::warn("Record overflow/underflow detected");
        return 1;
    }
    auto samples = static_cast<float*>(inputBuffer);
    std::lock_guard<std::mutex> lock(handler->getMutex());
    auto filterRes = handler->applyFilters(samples,nBufferFrames);
    const int endOfSentence = handler->checkForEndSentence(filterRes);
    if (endOfSentence) {
        AudioPacket audioPacket;
        audioPacket.type = AudioType::ENDOFSPEECH;
        audioPacket.samples = {};
        handler->pushAudioPacket(audioPacket);
        handler->resetSilenceCounter();
        handler->setLastAudioType(AudioType::ENDOFSPEECH);
        return 0;
    }
    if (filterRes == AudioType::SILENCE) {
        AudioPacket audioPacket;
        audioPacket.type = AudioType::SILENCE;
        if (handler->getSpeechCounter() > 0) {
            audioPacket.samples = std::vector<float>(samples, samples + nBufferFrames);
        } else {
            audioPacket.samples = {};
        }
        handler->pushAudioPacket(audioPacket);
    }
    else {
        int startSpeech = handler->checkForStartSentence();
        AudioPacket audioPacket;
        audioPacket.type = startSpeech == 1 ? AudioType::STARTOFSPEECH : AudioType::SPEECH;
        audioPacket.samples = std::vector<float>(samples,samples + nBufferFrames);
        handler->pushAudioPacket(audioPacket);
    }
    return 0;
}

int AudioHandler::playbackCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
    RtAudioStreamStatus status, void* userData) {


    auto* ctx = static_cast<PlaybackContext*>(userData);
    auto out = static_cast<int16_t*>(outputBuffer);
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->isTalking = true;
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        if (ctx->currentPos >= ctx->currentVector.size()) {
            if (!ctx->queue.empty()) {
                ctx->currentVector = std::move(ctx->queue.front());
                ctx->queue.pop();
                ctx->currentPos = 0;
            } else {
                out[i] = 0;
                continue;
            }
        }
        out[i] = ctx->currentVector.at(ctx->currentPos++);
    }
    ctx->isTalking = false;
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
    } catch( const std::exception& e )
    {
        SHOW_ERROR("Could not start audo stream " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }

}

void AudioHandler::init() {
    try {
        this->audio.openStream(nullptr,&this->parameters,
    RTAUDIO_FLOAT32,this->sampleRate,&this->bufferFrames,&AudioHandler::recordCallback,this);
        spdlog::info("RtAudio stream opened. Requested 512 frames, system allocated: {}", this->bufferFrames);
        this->filters.push_back(std::make_shared<SilenceFilter>(this->noiseThreshold));
    } catch( const std::exception& e ){
        SHOW_ERROR("Could not initialize audio handler " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }
}

AudioType AudioHandler::applyFilters(const float* samples, unsigned int nBufferFrames) {
    for (const auto& filter : this->filters) {
        if (filter->filter(samples,nBufferFrames)) {
            return filter->getActionType();
        }
    }
    return AudioType::NONE;
}

AudioPacket AudioHandler::getNextAudioPacket() {
    std::lock_guard<std::mutex> lock(this->queueMtx);
    auto packet = this->audioQueue.front();
    this->audioQueue.pop();
    return packet;
}

void AudioHandler::loadDeviceIds() {
    std::vector<unsigned int> ids = audio.getDeviceIds();

    if (ids.empty()) {
        SHOW_ERROR("Could not find any sound devices");
        return;
    }

    for (unsigned int id : ids) {
        try {
            RtAudio::DeviceInfo info = audio.getDeviceInfo(id);
            if (info.inputChannels > 0) {
                this->devices.emplace_back(id,info);
            }
        } catch (const std::exception& e) {
            continue;
        }
    }
}

void AudioHandler::startPlayback() {
    RtAudio::StreamParameters oParams;
    oParams.deviceId = this->playbackAudio.getDefaultOutputDevice();
    oParams.nChannels = 1;
    oParams.firstChannel = 0;

    unsigned int sampleRate_ = 22050;
    unsigned int bufferFrames_ = 512;
    this->playbackContext.currentPos = 0;
    try {
        if (!this->playbackAudio.isStreamOpen()) {
            this->playbackAudio.openStream(&oParams, nullptr, RTAUDIO_SINT16,
                                           sampleRate_, &bufferFrames_,
                                           &playbackCallback, &this->playbackContext);
        }

        this->playbackAudio.startStream();
        spdlog::info("INFO: Playback started at 22050Hz");
    } catch (const std::exception& e) {
        SHOW_ERROR("Could not start playback: " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }

}

bool AudioHandler::isSpeaking() const {
    return !playbackContext.queue.empty() || playbackContext.currentPos < playbackContext.currentVector.size();

}
