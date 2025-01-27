#include <stdio.h>

#include "raylib.h"

#include "common.hpp"
#include "util.hpp"
#include "drawing.hpp"

#include "game.hpp"

#include <iostream>
#include <cstring>
#include <cmath>
#include <map>
#include <string>
#include <chrono>
#include <algorithm>

int main()
{
    srand(static_cast<unsigned>(time(0))); // Inizializza il generatore casuale

    // Custom timming variables
    double previousTime = GetTime(); // Previous time measure
    double currentTime = 0.0;        // Current time measure
    double updateDrawTime = 0.0;     // Update + Draw time
    double waitTime = 0.0;           // Wait time (if target fps required)
    float deltaTime = 0.0f;          // Frame time (Update + Draw + Wait time)

    Game game;

    game.init();

    while (!WindowShouldClose())
    {
        game.updateAudioTrack();

        game.pollKeys();

        game.update(); // Chiama la funzione update per ogni intervallo di tempo fisso

        game.frame();

        currentTime = GetTime();
        updateDrawTime = currentTime - previousTime;

        waitTime = (1.0f / (float)game.getFPS()) - updateDrawTime;
        if (waitTime > 0.0)
        {
            WaitTime((float)waitTime);
            currentTime = GetTime();
            deltaTime = (float)(currentTime - previousTime);
        }

        previousTime = currentTime;
    }

    game.unloadAudioTrack(); // Unload music stream buffers from RAM

    game.destroy();

    CloseWindow();

    return 0;
}