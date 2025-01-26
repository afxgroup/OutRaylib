#include <stdio.h>

#include "raylib.h"

#include "constants.hpp"
#include "common.hpp"
#include "util.hpp"
#include "drawing.hpp"

#include <iostream>
#include <cstring>
#include <cmath>
#include <map>
#include <string>
#include <chrono>
#include <algorithm>

// Nome degli sprite (per debug o uso futuro)
const char *SPRITE_NAMES[] = {
    "PALM_TREE", "BILLBOARD08", "TREE1", "DEAD_TREE1", "BILLBOARD09", "BOULDER3",
    "COLUMN", "BILLBOARD01", "BILLBOARD06", "BILLBOARD05", "BILLBOARD07",
    "BOULDER2", "TREE2", "BILLBOARD04", "DEAD_TREE2", "BOULDER1",
    "BUSH1", "CACTUS", "BUSH2", "BILLBOARD03", "BILLBOARD02",
    "STUMP", "SEMI", "TRUCK", "CAR03", "CAR02", "CAR04",
    "CAR01", "PLAYER_UPHILL_LEFT", "PLAYER_UPHILL_STRAIGHT",
    "PLAYER_UPHILL_RIGHT", "PLAYER_LEFT", "PLAYER_STRAIGHT", "PLAYER_RIGHT"};

// Funzione per disegnare l'HUD
void RenderHUD(float pos, float spd)
{
    // Converti i valori in stringhe
    std::string positionText = "Position: " + std::to_string(static_cast<int>(pos));
    std::string speedText = "Speed: " + std::to_string(static_cast<int>(spd)) + " km/h";
    std::string maxSpeedText = "Max Speed: " + std::to_string(static_cast<int>(maxSpeed)) + " km/h";
    // Disegna lo sfondo dell'HUD
    DrawRectangle(10, 10, 300, 110, Fade(BLACK, 0.5f));

    // Disegna il testo
    DrawText(positionText.c_str(), 20, 20, 20, WHITE);
    DrawText(speedText.c_str(), 20, 50, 20, WHITE);
    DrawText(maxSpeedText.c_str(), 20, 80, 20, WHITE);
}

float lastY()
{
    if (segments.empty())
        return 0.0f;
    return segments.back().p2.world.y;
}

void addSegment(float curve, float y)
{
    int n = segments.size();
    Segment segment;

    segment.index = n;

    // Definizione del primo punto
    segment.p1.world.y = lastY();
    segment.p1.world.z = n * segmentLength;

    // Definizione del secondo punto
    segment.p2.world.y = y;
    segment.p2.world.z = (n + 1) * segmentLength;

    segment.curve = curve;

    // Alterna colori per il rumble strip
    segment.color = ((n / rumbleLength) % 2 == 0) ? DARK : LIGHT;

    // Aggiungi il segmento al vettore
    segments.push_back(segment);
}

// Funzione di utilità per verificare se due oggetti si sovrappongono
bool overlap(float x1, float w1, float x2, float w2, float percent = 1.0f)
{
    float half = percent / 2.0f;
    float min1 = x1 - (w1 * half);
    float max1 = x1 + (w1 * half);
    float min2 = x2 - (w2 * half);
    float max2 = x2 + (w2 * half);
    return !(max1 < min2 || min1 > max2);
}

Segment &findSegment(float z)
{
    int index = static_cast<int>(std::floor(z / segmentLength)) % segments.size();
    return segments[index];
}

// Funzione per aggiornare la posizione delle auto
void updateCars(float dt, Segment &playerSegment, float playerW)
{
    for (auto &car : cars)
    {
        // Trova il segmento attuale dell'auto
        Segment &oldSegment = findSegment(car.z);

        // Aggiorna l'offset in base al movimento dell'auto
        car.offset += updateCarOffset(car, oldSegment, playerSegment, playerW);

        // Aggiorna la posizione lungo il tracciato
        car.z = Util::increase(car.z, dt * car.speed, trackLength);

        // Calcola la percentuale rimanente per il rendering
        car.percent = Util::percentRemaining(car.z, segmentLength);

        // Trova il nuovo segmento dell'auto
        Segment &newSegment = findSegment(car.z);

        // Se l'auto è passata a un nuovo segmento, aggiorna i dati
        if (&oldSegment != &newSegment)
        {
            auto it = std::find(oldSegment.cars.begin(), oldSegment.cars.end(), car);
            if (it != oldSegment.cars.end())
            {
                oldSegment.cars.erase(it);
            }
            newSegment.cars.push_back(car);
        }
    }
}

float updateCarOffset(Car &car, Segment &carSegment, Segment &playerSegment, float playerW)
{
    const int lookahead = 20;  // Distanza di previsione
    float carW = car.sprite.w; // Larghezza dell'auto

    // Ottimizzazione: ignora le auto fuori dalla vista del giocatore
    if ((carSegment.index - playerSegment.index) > drawDistance)
    {
        return 0.0f;
    }

    for (int i = 1; i < lookahead; i++)
    {
        const Segment &segment = segments[(carSegment.index + i) % segments.size()];

        // Controllo collisione con il giocatore
        if (&segment == &playerSegment && car.speed > speed && overlap(playerX, playerW, car.offset, carW, 1.2f))
        {
            float dir = 0.0f;
            if (playerX > 0.5f)
            {
                dir = -1.0f;
            }
            else if (playerX < -0.5f)
            {
                dir = 1.0f;
            }
            else
            {
                dir = (car.offset > playerX) ? 1.0f : -1.0f;
            }
            return dir * (1.0f / i) * (car.speed - speed) / maxSpeed;
        }

        // Controllo collisione con altre auto
        for (const Car &otherCar : segment.cars)
        {
            float otherCarW = otherCar.sprite.w;
            if (car.speed > otherCar.speed && overlap(car.offset, carW, otherCar.offset, otherCarW, 1.2f))
            {
                float dir = 0.0f;
                if (otherCar.offset > 0.5f)
                {
                    dir = -1.0f;
                }
                else if (otherCar.offset < -0.5f)
                {
                    dir = 1.0f;
                }
                else
                {
                    dir = (car.offset > otherCar.offset) ? 1.0f : -1.0f;
                }
                return dir * (1.0f / i) * (car.speed - otherCar.speed) / maxSpeed;
            }
        }
    }

    // Se l'auto è fuori strada, correggi l'offset
    if (car.offset < -0.9f)
    {
        return 0.1f;
    }
    else if (car.offset > 0.9f)
    {
        return -0.1f;
    }
    else
    {
        return 0.0f;
    }
}

void addSprite(int n, Sprite sprite, float offset)
{
    if (n >= 0 && n < segments.size())
    {
        sprite.source = {sprite.x, sprite.y, sprite.w, sprite.h};
#if 0
        sprite.x = 0;
        sprite.y = 0;
        sprite.w = 0;
        sprite.h = 0;
#endif
        sprite.offset = offset;
        // Aggiungi lo sprite al segmento
        segments[n].sprites.push_back(sprite);
    }
}

void addRoad(int enter, int hold, int leave, float curve, float y)
{
    // Ottieni l'altezza iniziale (startY) e calcola l'altezza finale (endY)
    float startY = lastY();
    float endY = startY + (static_cast<int>(y) * segmentLength); // Utilizzo di y per calcolare la nuova altezza

    int total = enter + hold + leave; // Numero totale di segmenti

    // Aggiungi segmenti per la fase di entrata
    for (int n = 0; n < enter; n++)
    {
        float segmentCurve = Util::easeIn(0.0f, curve, static_cast<float>(n) / enter);
        float segmentY = Util::easeInOut(startY, endY, static_cast<float>(n) / total);
        addSegment(segmentCurve, segmentY);
    }

    // Aggiungi segmenti per la fase centrale (costante)
    for (int n = 0; n < hold; n++)
    {
        float segmentY = Util::easeInOut(startY, endY, static_cast<float>(enter + n) / total);
        addSegment(curve, segmentY);
    }

    // Aggiungi segmenti per la fase di uscita
    for (int n = 0; n < leave; n++)
    {
        float segmentCurve = Util::easeInOut(curve, 0.0f, static_cast<float>(n) / leave);
        float segmentY = Util::easeInOut(startY, endY, static_cast<float>(enter + hold + n) / total);
        addSegment(segmentCurve, segmentY);
    }
}

// Funzione per aggiungere un tratto rettilineo
void addStraight(int num = ROAD::LENGTH::MEDIUM)
{
    addRoad(num, num, num, 0.0f, 0.0f);
}

// Funzione per aggiungere una collina
void addHill(int num = ROAD::LENGTH::MEDIUM, int _height = ROAD::HILL::MEDIUM)
{
    addRoad(num, num, num, 0.0f, static_cast<float>(_height));
}

// Funzione per aggiungere una curva
void addCurve(int num = ROAD::LENGTH::MEDIUM, int curve = ROAD::CURVE::MEDIUM, int _height = ROAD::HILL::NONE)
{
    addRoad(num, num, num, static_cast<float>(curve), static_cast<float>(_height));
}

// Funzione per aggiungere colline basse ondulate
void addLowRollingHills(int num = ROAD::LENGTH::SHORT, int _height = ROAD::HILL::LOW)
{
    addRoad(num, num, num, 0.0f, static_cast<float>(_height) / 2.0f);
    addRoad(num, num, num, 0.0f, -static_cast<float>(_height));
    addRoad(num, num, num, static_cast<float>(ROAD::CURVE::EASY), static_cast<float>(_height));
    addRoad(num, num, num, 0.0f, 0.0f);
    addRoad(num, num, num, -static_cast<float>(ROAD::CURVE::EASY), static_cast<float>(_height) / 2.0f);
    addRoad(num, num, num, 0.0f, 0.0f);
}

// Funzione per aggiungere curve a forma di S
void addSCurves()
{
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::EASY, ROAD::HILL::NONE);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::CURVE::MEDIUM, ROAD::HILL::MEDIUM);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::CURVE::EASY, -ROAD::HILL::LOW);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::EASY, ROAD::HILL::MEDIUM);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::MEDIUM, -ROAD::HILL::MEDIUM);
}

// Funzione per aggiungere dossi
void addBumps()
{
    addRoad(10, 10, 10, 0.0f, 5.0f);
    addRoad(10, 10, 10, 0.0f, -2.0f);
    addRoad(10, 10, 10, 0.0f, -5.0f);
    addRoad(10, 10, 10, 0.0f, 8.0f);
    addRoad(10, 10, 10, 0.0f, 5.0f);
    addRoad(10, 10, 10, 0.0f, -7.0f);
    addRoad(10, 10, 10, 0.0f, 5.0f);
    addRoad(10, 10, 10, 0.0f, -2.0f);
}

// Funzione per aggiungere una discesa fino alla fine
void addDownhillToEnd(int num = 200)
{
    addRoad(num, num, num, -ROAD::CURVE::EASY, -lastY() / segmentLength);
}

void resetSprites()
{
    std::vector<float> choices = {1.0f, -1.0f};
    // Aggiungi sprite fissi iniziali
    addSprite(20, SPRITES::BILLBOARD07, -1.0f);
    addSprite(40, SPRITES::BILLBOARD06, -1.0f);
    addSprite(60, SPRITES::BILLBOARD08, -1.0f);
    addSprite(80, SPRITES::BILLBOARD09, -1.0f);
    addSprite(100, SPRITES::BILLBOARD01, -1.0f);
    addSprite(120, SPRITES::BILLBOARD02, -1.0f);
    addSprite(140, SPRITES::BILLBOARD03, -1.0f);
    addSprite(160, SPRITES::BILLBOARD04, -1.0f);
    addSprite(180, SPRITES::BILLBOARD05, -1.0f);

    // Aggiungi sprite ai lati del tracciato
    addSprite(240, SPRITES::BILLBOARD07, -1.2f);
    addSprite(240, SPRITES::BILLBOARD06, 1.2f);
    addSprite(segments.size() - 25, SPRITES::BILLBOARD07, -1.2f);
    addSprite(segments.size() - 25, SPRITES::BILLBOARD06, 1.2f);

    // Aggiungi sprite a intervalli casuali
    for (int n = 10; n < 200; n += 4 + n / 100)
    {
        addSprite(n, SPRITES::PALM_TREE, 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f);
        addSprite(n, SPRITES::PALM_TREE, 1.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f);
    }

    // Aggiungi colonne e alberi
    for (int n = 250; n < 1000; n += 5)
    {
        addSprite(n, SPRITES::COLUMN, 1.1f);
        addSprite(n + Util::randomInt(0, 5), SPRITES::TREE1, -1.0f - static_cast<float>(rand()) / RAND_MAX * 2.0f);
        addSprite(n + Util::randomInt(0, 5), SPRITES::TREE2, -1.0f - static_cast<float>(rand()) / RAND_MAX * 2.0f);
    }

    // Aggiungi piante
    for (int n = 200; n < segments.size(); n += 3)
    {
        addSprite(n, Util::randomChoice(PLANTS), Util::randomChoice(choices) * (2.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f));
    }

    // Aggiungi sprite complessi
    for (int n = 1000; n < static_cast<int>(segments.size()) - 50; n += 100)
    {
        float side = Util::randomChoice(choices);
        addSprite(n + Util::randomInt(0, 50), Util::randomChoice(BILLBOARDS), -side);

        for (int i = 0; i < 20; ++i)
        {
            Sprite sprite = Util::randomChoice(PLANTS);
            float offset = side * (1.5f + static_cast<float>(rand()) / RAND_MAX);
            addSprite(n + Util::randomInt(0, 50), sprite, offset);
        }
    }
}

void resetRoad()
{
    // Svuota l'elenco dei segmenti
    segments.clear();

    // Aggiungi i vari tratti della strada
    addStraight(ROAD::LENGTH::SHORT);
    addLowRollingHills();
    addSCurves();
    addCurve(ROAD::LENGTH::MEDIUM, ROAD::CURVE::MEDIUM, ROAD::HILL::LOW);
    addBumps();
    addLowRollingHills();
    addCurve(ROAD::LENGTH::LONG * 2, ROAD::CURVE::MEDIUM, ROAD::HILL::MEDIUM);
    addStraight();
    addHill(ROAD::LENGTH::MEDIUM, ROAD::HILL::HIGH);
    addSCurves();
    addCurve(ROAD::LENGTH::LONG, -ROAD::CURVE::MEDIUM, ROAD::HILL::NONE);
    addHill(ROAD::LENGTH::LONG, ROAD::HILL::HIGH);
    addCurve(ROAD::LENGTH::LONG, ROAD::CURVE::MEDIUM, -ROAD::HILL::LOW);
    addBumps();
    addHill(ROAD::LENGTH::LONG, -ROAD::HILL::MEDIUM);
    addStraight();
    addSCurves();
    addDownhillToEnd();

    // Resetta gli sprite e le auto

    resetSprites();
    resetCars();

    // Configura il colore dei segmenti di partenza
    int startIndex = findSegment(playerZ).index;
    segments[startIndex + 2].color = START;
    segments[startIndex + 3].color = START;

    // Configura il colore dei segmenti di arrivo
    for (int n = 0; n < rumbleLength; n++)
    {
        segments[segments.size() - 1 - n].color = FINISH;
    }

    // Calcola la lunghezza totale del tracciato
    trackLength = segments.size() * segmentLength;
}

void resetCars()
{
    cars.clear();
    float _speed;

    for (int n = 0; n < totalCars; n++)
    {
        // Calcola l'offset casuale e scegli un lato casuale
        float offset = static_cast<float>(rand()) / RAND_MAX * Util::randomChoice(std::vector<float>{-0.8f, 0.8f});

        // Calcola la posizione z casuale
        float z = static_cast<float>(rand()) / RAND_MAX * static_cast<float>(segments.size()) * segmentLength;

        // Seleziona uno sprite casuale
        Sprite sprite = Util::randomChoice(CARS);

        float c = 2.0f;
        if (sprite.h == SPRITES::SEMI.h &&
            sprite.y == SPRITES::SEMI.y &&
            sprite.x == SPRITES::SEMI.x &&
            sprite.w == SPRITES::SEMI.w)
            c = 4.0f;
        // Calcola la velocità dell'auto
        _speed = maxSpeed / 4.0f +
                 static_cast<float>(rand()) / RAND_MAX * maxSpeed / c;

        // Crea l'auto
        Car car = {n, offset, z, sprite, _speed};

        // Trova il segmento corrispondente e aggiungi l'auto
        Segment &segment = findSegment(car.z);
        segment.cars.push_back(car);

        // Aggiungi l'auto alla lista globale
        cars.push_back(car);
    }
}

std::string formatTime(float dt)
{
    int minutes = static_cast<int>(std::floor(dt / 60));
    int seconds = static_cast<int>(std::floor(dt - (minutes * 60)));
    int tenths = static_cast<int>(std::floor(10 * (dt - std::floor(dt))));

    if (minutes > 0)
    {
        return std::to_string(minutes) + "." +
               (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
               std::to_string(tenths);
    }
    else
    {
        return std::to_string(seconds) + "." + std::to_string(tenths);
    }
}

void reset(std::map<std::string, int> options)
{
    // Leggi o usa i valori di default
    width = options.count("width") ? options["width"] : SCREEN_WIDTH;
    height = options.count("height") ? options["height"] : SCREEN_HEIGHT;
    lanes = options.count("lanes") ? options["lanes"] : 3;
    roadWidth = options.count("roadWidth") ? options["roadWidth"] : 2000.0f;
    cameraHeight = options.count("cameraHeight") ? options["cameraHeight"] : 1000.0f;
    drawDistance = options.count("drawDistance") ? options["drawDistance"] : 300;
    fogDensity = options.count("fogDensity") ? options["fogDensity"] : 5.0f;
    fieldOfView = options.count("fieldOfView") ? options["fieldOfView"] : 100.0f;
    segmentLength = options.count("segmentLength") ? options["segmentLength"] : 200.0f;
    rumbleLength = options.count("rumbleLength") ? options["rumbleLength"] : 3;

    // Calcoli aggiuntivi
    cameraDepth = 1 / std::tan((fieldOfView / 2.0f) * (M_PI / 180.0f));
    playerZ = cameraHeight * cameraDepth;
    resolution = static_cast<float>(height) / 480.0f;

    // Ricostruisci la strada se necessario
    if (segments.empty() || options.count("segmentLength") || options.count("rumbleLength"))
    {
        resetRoad();
    }
}

void update(float dt)
{
    // Variabili locali
    Segment &playerSegment = findSegment(position + playerZ);
    float playerW = SPRITES::PLAYER_STRAIGHT.w * SPRITE_SCALE;
    float speedPercent = speed / maxSpeed;
    float dx = dt * 2.0f * speedPercent; // Velocità laterale massima
    float startPosition = position;

    // Aggiorna le auto
    updateCars(dt, playerSegment, playerW);

    // Aggiorna la posizione lungo il tracciato
    position = Util::increase(position, dt * speed, trackLength);

    // Movimento laterale del giocatore
    if (keyLeft)
    {
        playerX -= dx;
    }
    else if (keyRight)
    {
        playerX += dx;
    }

    // Effetto centrifugo
    playerX -= dx * speedPercent * playerSegment.curve * centrifugal;

    // Aggiorna la velocità del giocatore
    if (keyFaster)
    {
        speed = Util::accelerate(speed, accel, dt);
    }
    else if (keySlower)
    {
        speed = Util::accelerate(speed, breaking, dt);
    }
    else
    {
        speed = Util::accelerate(speed, decel, dt);
    }

    // Controlla se il giocatore è fuori strada
    if (playerX < -1.0f || playerX > 1.0f)
    {
        if (speed > offRoadLimit)
        {
            speed = Util::accelerate(speed, offRoadDecel, dt);
        }

        // Controlla collisioni con sprite
        for (const auto &sprite : playerSegment.sprites)
        {
            float spriteW = sprite.w * SPRITE_SCALE;
            if (Util::overlap(playerX, playerW, sprite.offset + spriteW / 2.0f * (sprite.offset > 0 ? 1 : -1), spriteW))
            {
                speed = maxSpeed / 5.0f;
                position = Util::increase(playerSegment.p1.world.z, -playerZ, trackLength);
                break;
            }
        }
    }

    // Controlla collisioni con altre auto
    for (const auto &car : playerSegment.cars)
    {
        float carW = car.sprite.w * SPRITE_SCALE;
        if (speed > car.speed)
        {
            if (Util::overlap(playerX, playerW, car.offset, carW, 0.8f))
            {
                speed = car.speed * (car.speed / speed);
                position = Util::increase(car.z, -playerZ, trackLength);
                break;
            }
        }
    }

    // Limita la posizione laterale e la velocità del giocatore
    playerX = Util::limit(playerX, -3.0f, 3.0f);
    speed = Util::limit(speed, 0.0f, maxSpeed);

    // Aggiorna gli offset per lo sfondo
    skyOffset = Util::increase(skyOffset, skySpeed * playerSegment.curve * (position - startPosition) / segmentLength, 1.0f);
    hillOffset = Util::increase(hillOffset, hillSpeed * playerSegment.curve * (position - startPosition) / segmentLength, 1.0f);
    treeOffset = Util::increase(treeOffset, treeSpeed * playerSegment.curve * (position - startPosition) / segmentLength, 1.0f);

    if (position > playerZ)
    {
        if (currentLapTime && (startPosition < playerZ))
        {
            lastLapTime = currentLapTime;
            currentLapTime = 0;
#if 0          
          if (lastLapTime <= Util::toFloat(Dom.storage.fast_lap_time)) {
            Dom.storage.fast_lap_time = lastLapTime;
            updateHud('fast_lap_time', formatTime(lastLapTime));
            Dom.addClassName('fast_lap_time', 'fastest');
            Dom.addClassName('last_lap_time', 'fastest');
          }
          else {
            Dom.removeClassName('fast_lap_time', 'fastest');
            Dom.removeClassName('last_lap_time', 'fastest');
          }
          updateHud('last_lap_time', formatTime(lastLapTime));
          Dom.show('last_lap_time');
#endif
        }
        else
        {
            currentLapTime += dt;
        }
    }
}

// Funzione per ottenere il timestamp corrente in millisecondi
float getTimestamp()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0f; // Ritorna in secondi
}

int main()
{
    srand(static_cast<unsigned>(time(0))); // Inizializza il generatore casuale

    // Inizializzazione della finestra
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OutRaylib");
    SetTargetFPS(60);

    InitAudioDevice();              // Initialize audio device

    std::map<std::string, int> options = {};
    reset(options);

    // Dom.storage.fast_lap_time = Dom.storage.fast_lap_time || 180;
    // UpdateHud('fast_lap_time', formatTime(Util.toFloat(Dom.storage.fast_lap_time)));

    Image image = LoadImage("resources/images/background.png"); // Loaded in CPU memory (RAM)
    background = LoadTextureFromImage(image);                   // Image converted to texture, GPU memory (VRAM)
    image = LoadImage("resources/images/sprites.png");
    sprites = LoadTextureFromImage(image);

    Music music = LoadMusicStream("resources/music/music.mp3");
    PlayMusicStream(music);

    while (!WindowShouldClose())
    {
        int n, i;
        Sprite sprite;
        Car car;
        float spriteScale, spriteX, spriteY;
        
        float now = 0.0f;
        float last = 0.0f;
        float gdt = 0.0f;
        float dt = 0.0f;

        UpdateMusicStream(music);   // Update music buffer with new stream data

        keyLeft = keyRight = keyFaster = keySlower = false;
        if (IsKeyDown(KEY_RIGHT))
            keyRight = true;
        else
            keyRight = false;

        if (IsKeyDown(KEY_LEFT))
            keyLeft = true;
        else
            keyLeft = false;

        if (IsKeyDown(KEY_UP))
            keyFaster = true;
        else
            keyFaster = false;

        if (IsKeyDown(KEY_DOWN))
            keySlower = true;
        else
            keySlower = false;

        Segment& baseSegment = findSegment(position);
        float basePercent = Util::percentRemaining(position, segmentLength);
        Segment& playerSegment = findSegment(position + playerZ);
        float playerPercent = Util::percentRemaining(position + playerZ, segmentLength);
        float playerY = Util::interpolate(playerSegment.p1.world.y, playerSegment.p2.world.y, playerPercent);
        int maxy = height;

        int x = 0;
        float dx = -(baseSegment.curve * basePercent);

        // printf("%d %f %d %f %f %d %f\n", baseSegment.index, basePercent, playerSegment.index, playerPercent, playerY, maxy, dx);

        now = getTimestamp();
        dt = std::min(1.0f, (now - last)); // Limita dt a un massimo di 1 secondo
        gdt += dt;

        while (gdt > step)
        {
            gdt -= step;
            update(step); // Chiama la funzione update per ogni intervallo di tempo fisso
        }

        // Rendering
        BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawBackground(background, width, height, BACKGROUND::SKY, skyOffset, resolution * skySpeed * playerY);
        DrawBackground(background, width, height, BACKGROUND::HILLS, hillOffset, resolution * hillSpeed * playerY);
        DrawBackground(background, width, height, BACKGROUND::TREES, treeOffset, resolution * treeSpeed * playerY);

        for (n = 0; n < drawDistance; n++)
        {
            Segment& segment = segments[(baseSegment.index + n) % segments.size()];
            segment.looped = segment.index < baseSegment.index;
            segment.fog = Util::exponentialFog(n / drawDistance, fogDensity);
            segment.clip = maxy;

            Util::project(segment.p1, (playerX * roadWidth) - x, playerY + cameraHeight, position - (segment.looped ? trackLength : 0), cameraDepth, width, height, roadWidth);
            Util::project(segment.p2, (playerX * roadWidth) - x - dx, playerY + cameraHeight, position - (segment.looped ? trackLength : 0), cameraDepth, width, height, roadWidth);

            //printf("%d %f %f %f %f\n", n, segment.p1.screen.x, segment.p1.screen.y, segment.p1.screen.w, segment.p1.screen.scale);

            x = x + dx;
            dx = dx + segment.curve;

            if ((segment.p1.camera.z <= cameraDepth) ||         // behind us
                (segment.p2.screen.y >= segment.p1.screen.y) || // back face cull
                (segment.p2.screen.y >= maxy))                  // clip by (already rendered) hill
                continue;

            DrawSegment(width, lanes,
                        segment.p1.screen.x,
                        segment.p1.screen.y,
                        segment.p1.screen.w,
                        segment.p2.screen.x,
                        segment.p2.screen.y,
                        segment.p2.screen.w,
                        segment.fog,
                        segment.color);

            maxy = segment.p1.screen.y;
        }

        for (n = (drawDistance - 1); n > 0; n--)
        {
            Segment& segment = segments[(baseSegment.index + n) % segments.size()];

            for (i = 0; i < segment.cars.size(); i++)
            {
                car = segment.cars[i];
                sprite = car.sprite;
                spriteScale = Util::interpolate(segment.p1.screen.scale, segment.p2.screen.scale, car.percent);
                spriteX = Util::interpolate(segment.p1.screen.x, segment.p2.screen.x, car.percent) + (spriteScale * car.offset * roadWidth * width / 2);
                spriteY = Util::interpolate(segment.p1.screen.y, segment.p2.screen.y, car.percent);
                DrawSprite(sprites, width, height, resolution, roadWidth, sprite, spriteScale, spriteX, spriteY, -0.5, -1, segment.clip);
            }

            for (i = 0; i < segment.sprites.size(); i++)
            {
                sprite = segment.sprites[i];
                spriteScale = segment.p1.screen.scale;
                spriteX = segment.p1.screen.x + (spriteScale * sprite.offset * roadWidth * width / 2);
                spriteY = segment.p1.screen.y;
                DrawSprite(sprites, width, height, resolution, roadWidth, sprite, spriteScale, spriteX, spriteY, (sprite.offset < 0 ? -1 : 0), -1, segment.clip);
            }

            if (segment.index == playerSegment.index)
            {
                DrawPlayer(sprites, width, height, resolution, roadWidth, speed / maxSpeed,
                           cameraDepth / playerZ,
                           width / 2,
                           (height / 2) - (cameraDepth / playerZ * Util::interpolate(playerSegment.p1.camera.y, playerSegment.p2.camera.y, playerPercent) * height / 2),
                           speed * (keyLeft ? -1 : keyRight ? 1
                                                            : 0),
                           playerSegment.p2.world.y - playerSegment.p1.world.y);
            }
        }

        // Draw HUD
        DrawRectangle(0, 0, width, 60, Color{0xFF, 0x00, 0x00, 127});
        DrawRectangleLines(0, 0, width, 60, BLACK);
        
        // Time
        DrawRectangle(10, 10, 100, 40, Color{0xFF, 0xFF, 0xFF, 127});
        DrawRectangleLines(10, 10, 100, 40, BLACK);

        // Speed
        DrawRectangle(width - 110, 10, 100, 40, Color{0xFF, 0xFF, 0xFF, 127});
        DrawRectangleLines(width - 110, 10, 100, 40, BLACK);
        
        // Fastest Lap
        DrawRectangle(width / 2 - 100, 10, 200, 40, Color{0xFF, 0xFF, 0xFF, 127});
        DrawRectangleLines(width / 2 - 100, 10, 200, 40, BLACK);

        UpdateHud("speed",            std::to_string(static_cast<int>(5 * round(speed/500))), width);
        UpdateHud("current_lap_time", formatTime(currentLapTime), width);    

        EndDrawing();

        last = now;
    }

    UnloadMusicStream(music);   // Unload music stream buffers from RAM

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)
    
    CloseWindow();

    return 0;
}