#include "Sound.hpp"
#include <iostream>

Sound::Sound(std::string filePath){
    if(SDL_LoadWAV(filePath.c_str(), &m_audioSpec, &m_waveStart, &m_waveLength) == nullptr){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Sound loading error");
    }
}

Sound::~Sound() {
    SDL_FreeWAV(m_waveStart);
    SDL_CloseAudioDevice(m_device);
}

void Sound::play(){
    SDL_ClearQueuedAudio(m_device);
    int status = SDL_QueueAudio(m_device, m_waveStart, m_waveLength);
    SDL_PauseAudioDevice(m_device, 0);
}

void Sound::stop(){
    SDL_PauseAudioDevice(m_device, 1);
}

void Sound::setupDevice(){
    m_device = SDL_OpenAudioDevice(nullptr, 0, &m_audioSpec, nullptr, 0);
    if(m_device == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Sound device error");
    }
}