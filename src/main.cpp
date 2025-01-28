#include "raylib.h"

#include "common.hpp"
#include "util.hpp"

#include "game.hpp"

int main()
{
    srand(static_cast<unsigned>(time(0))); // Inizializza il generatore casuale

    Game game;

    game.init();

    while (!WindowShouldClose())
    {
        game.pollKeys();

        if (!game.isPaused()) {
            game.updateAudioTrack();

            game.update(); // Chiama la funzione update per ogni intervallo di tempo fisso
        }

        game.frame();
    }

    game.unloadAudioTrack(); // Unload music stream buffers from RAM

    game.destroy();

    CloseWindow();

    return 0;
}