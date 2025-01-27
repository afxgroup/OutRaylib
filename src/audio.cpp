#include "audio.hpp"

/* Music functions */
void Audio::init()
{
    InitAudioDevice(); // Initialize audio device
}

void Audio::destroy()
{
    CloseAudioDevice(); // Close audio device (music streaming is automatically stopped)
}

void Audio::loadTrack(const char *filename)
{
    track = LoadMusicStream(filename);
}

void Audio::playTrack()
{
    PlayMusicStream(track);
}

void Audio::updateTrack()
{
    UpdateMusicStream(track); // Update music buffer with new stream data
}

void Audio::unloadTrack()
{
    UnloadMusicStream(track);
}

void Audio::toggleAudio()
{
    muted = !muted;
    if (muted)
        SetMusicVolume(track, 0);
    else
        SetMusicVolume(track, 1.0f);
}