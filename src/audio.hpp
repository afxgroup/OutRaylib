#ifndef __AUDIO_HPP__
#define __AUDIO_HPP__

#include "raylib.h"

class Audio
{
public:
    void init();
    void destroy();

    void loadTrack(const char *filename);
    void playTrack();
    void updateTrack();
    void unloadTrack();
    void toggleAudio();

private:
    Music track;
    bool muted = false;
};

#endif