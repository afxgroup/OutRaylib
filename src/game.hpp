#ifndef __GAME_HPP__
#define __GAME_HPP__

#include "raylib.h"

#include <iostream>
#include <fstream>

#include "common.hpp"
#include "util.hpp"

#include "drawing.hpp"
#include "audio.hpp"

#include <nlohmann/json.hpp>

class Game
{
public:
    void init();
    void destroy();

    void update();
    void updateHUD(std::string key, std::string value);
    void frame();
    void pollKeys();

    void loadOptions(std::map<std::string, int> options);

    void updateAudioTrack();
    void unloadAudioTrack();

    int getFPS() { return fps; }

private:
    Font fontTtf;
    Audio audio;
    Drawing drawing;
    float fastestLapTime = 0.0f; // Miglior tempo
    // Stato della tastiera
    bool keyLeft = false;
    bool keyRight = false;
    bool keyFaster = false;
    bool keySlower = false;

    const int fps = 60;            // Frame per secondo
    const float step = 1.0f / fps; // Durata di ogni frame (in secondi)

    const float centrifugal = 0.3f; // Moltiplicatore di forza centrifuga
    const float skySpeed = 0.001f;  // Velocità di scorrimento dello sfondo (cielo)
    const float hillSpeed = 0.002f; // Velocità di scorrimento dello sfondo (colline)
    const float treeSpeed = 0.003f; // Velocità di scorrimento dello sfondo (alberi)

    float skyOffset = 0.0f;                // Offset attuale dello sfondo (cielo)
    float hillOffset = 0.0f;               // Offset attuale dello sfondo (colline)
    float treeOffset = 0.0f;               // Offset attuale dello sfondo (alberi)
    std::vector<Segment> segments;         // Array di segmenti stradali
    std::vector<Car> cars;                 // Array di auto sulla strada
    void *stats = nullptr;                 // Placeholder per un contatore FPS (es. Mr. Doob's)
    void *canvas = nullptr;                // Placeholder per il canvas
    void *ctx = nullptr;                   // Placeholder per il contesto grafico
    Texture2D background;                  // Immagine di sfondo (da caricare)
    Texture2D sprites;                     // Spritesheet (da caricare)
    float resolution = 0.0f;               // Fattore di scaling per risoluzione indipendente
    float roadWidth = 2000.0f;             // Larghezza della strada
    float segmentLength = 200.0f;          // Lunghezza di un segmento
    int rumbleLength = 3;                  // Numero di segmenti per una striscia rossa/bianca
    float trackLength = 0.0f;              // Lunghezza totale del tracciato (calcolata)
    int lanes = 3;                         // Numero di corsie
    float fieldOfView = 100.0f;            // Angolo del campo visivo (in gradi)
    float cameraHeight = 1000.0f;          // Altezza della telecamera
    float cameraDepth = 0.0f;              // Distanza Z della telecamera (calcolata)
    int drawDistance = 300;                // Numero di segmenti da disegnare
    float playerX = 0.0f;                  // Offset X del giocatore (-1 a 1)
    float playerZ = 0.0f;                  // Distanza Z relativa del giocatore (calcolata)
    float fogDensity = 5.0f;               // Densità della nebbia
    float position = 0.0f;                 // Posizione attuale della telecamera lungo l'asse Z
    float speed = 0.0f;                    // Velocità attuale
    float maxSpeed = segmentLength / step; // Velocità massima
    float accel = maxSpeed / 5.0f;         // Accelerazione
    float breaking = -maxSpeed;            // Decelerazione durante la frenata
    float decel = -maxSpeed / 5.0f;        // Decelerazione naturale
    float offRoadDecel = -maxSpeed / 2.0f; // Decelerazione fuori strada
    float offRoadLimit = maxSpeed / 4.0f;  // Velocità minima fuori strada
    int totalCars = 200;                   // Numero totale di auto sulla strada
    float currentLapTime = 0.0f;           // Tempo attuale del giro
    float lastLapTime = 0.0f;              // Ultimo tempo del giro
    int width = 1024;                      // Larghezza logica del canvas
    int height = 768;                      // Altezza logica del canvas

    void loadImages();
    void saveScore();
    void loadScore();
    float lastY();

    void renderHUD();

    void addSprite(int n, Sprite sprite, float offset);
    void addRoad(int enter, int hold, int leave, float curve, float y);
    void addStraight(int num);
    void addHill(int num, int _height);
    void addCurve(int num, int curve, int _height);
    void addLowRollingHills(int num, int _height);
    void addSCurves();
    void addBumps();
    void addDownhillToEnd(int num);

    void resetSprites();
    void resetRoad();
    void resetCars();

    void updateCars(float dt, Segment &playerSegment, float playerW);
    float updateCarOffset(Car &car, Segment &carSegment, Segment &playerSegment, float playerW);

    void addSegment(float curve, float y);
    Segment &findSegment(float z);
};

#endif