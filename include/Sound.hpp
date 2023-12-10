#ifndef SOUND_HPP
#define SOUND_HPP

#include <string>

#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

class Sound {
public:
    Sound(std::string filePath);
    ~Sound();
    void play();
    void stop();
    void setupDevice();

private:
    SDL_AudioDeviceID m_device;
    SDL_AudioSpec m_audioSpec;
    Uint8* m_waveStart;
    Uint32 m_waveLength;
};

#endif
