//
// Created by andrew on 01.03.26.
//

#include "../headers/AudioHandler.h"

#include "../headers/SilenceFilter.h"

AudioHandler::AudioHandler(unsigned int noChannels,unsigned int firstChanel,unsigned int sampleRate, unsigned int bufferFrames) {
    this->loadDeviceIds();

    unsigned int defaultId = this->audio.getDefaultInputDevice();

    if (defaultId == 0 && this->audio.getDeviceCount() == 0) {
        throw std::runtime_error("Could not find sound devices");
    }

    std::cout << "DEBUG: Default Input Device ID: " << defaultId << std::endl;

    this->parameters.deviceId = defaultId;
    this->parameters.nChannels = noChannels;
    this->parameters.firstChannel = firstChanel;

    this->bufferFrames = bufferFrames;
    this->sampleRate = sampleRate;

    this->init();
}

int AudioHandler::checkForEndSentence(AudioType type) {
    if (type == AudioType::SILENCE) {
        this->silenceCounter++;
    } else {
        this->silenceCounter = 0;
        return 0;
    }

    const int endSentenceThreshold = 20;

    if (this->silenceCounter >= endSentenceThreshold && this->speechCounter > 0) {
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
        std::cout << "Stream over/underflow detected." << std::endl;
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
        audioPacket.samples = {};
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
        std::cerr << "Error when starting audio stream." << std::endl;
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

}

void AudioHandler::init() {
    try {
        this->audio.openStream(nullptr,&this->parameters,
    RTAUDIO_FLOAT32,this->sampleRate,&this->bufferFrames,&AudioHandler::recordCallback,this);
        this->filters.push_back(std::make_shared<SilenceFilter>(0.07));
    } catch( const std::exception& e ){
        std::cerr << "Error when initializing audio stream." << std::endl;
        std::cerr << e.what() << std::endl;
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
        std::cerr << "Could not find any sound devices" << std::endl;
        return;
    }

    for (unsigned int id : ids) {
        try {
            RtAudio::DeviceInfo info = audio.getDeviceInfo(id);
            if (info.inputChannels > 0) {
                std::cout << "Found microphone - ID: " << id << " Name: " << info.name << std::endl;
                this->devices.push_back({id,info});

            }
        } catch (const std::exception& e) {
            continue;
        }
    }
}
