#include "Sound.hpp"
#include <iostream>

Sound::Sound(std::string filePath){
    if(SDL_LoadWAV(filePath.c_str(), &m_audioSpec, &m_waveStart, &m_waveLength) == nullptr){
        std::cerr << "Sound Loading Failed: " << SDL_GetError() << std::endl;
        SDL_Log("Sound loading error");
    }else{
        SDL_Log("Sound loaded: %s", filePath.c_str());
    }
}

Sound::~Sound() {
    SDL_FreeWAV(m_waveStart);
    SDL_CloseAudioDevice(m_device);
}

void Sound::play(){
    int status = SDL_QueueAudio(m_device, m_waveStart, m_waveLength);
    SDL_PauseAudioDevice(m_device, 0);
}

void Sound::stop(){
    SDL_ClearQueuedAudio(m_device);
}

void Sound::setupDevice(){
    m_device = SDL_OpenAudioDevice(nullptr, 0, &m_audioSpec, nullptr, 0);
    if(m_device == 0)
        // std::cerr << "Sound device error" << SDL_GetError() << std::endl;
        SDL_Log("Sound device error");
}