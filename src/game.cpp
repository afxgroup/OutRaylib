#include "game.hpp"

using json = nlohmann::json;

void Game::loadScore()
{
    std::ifstream f("score.json");
    if (f.good())
    {
        json data = json::parse(f);
        fastestLapTime = static_cast<float>(data["fast_lap_time"]);
    }
}

void Game::saveScore()
{
    std::ofstream f("score.json");
    if (f.good())
    {
        json j;
        j["fast_lap_time"] = fastestLapTime;
        f << j << std::endl;
        f.close();
    }
}

// Funzione di utilità per verificare se due oggetti si sovrappongono
void Game::init()
{
    std::map<std::string, std::string> options = {};
    std::ifstream f("options.json");
    if (f.good())
    {
        json data = json::parse(f);
        options = data.get<std::map<std::string, std::string>>();
    }
    loadOptions(options);

    // Inizializzazione della finestra
    InitWindow(width, height, "OutRaylib");
    SetTargetFPS(fps);

    loadImages();

    loadScore();

    audio.init();
    audio.loadTrack(tracks[0].c_str());
    audio.playTrack();

    fontTtf = LoadFontEx("resources/font/Retroica.ttf", 24, 0, 250);
}

void Game::destroy()
{
    UnloadTexture(background);
    UnloadTexture(sprites);

    audio.destroy();
    UnloadFont(fontTtf);
}

void Game::updateAudioTrack()
{
    audio.updateTrack();
}

void Game::unloadAudioTrack()
{
    audio.unloadTrack();
}

void Game::update()
{
    Segment &playerSegment = findSegment(position + playerZ);
    float playerW = SPRITES::PLAYER_STRAIGHT.w * SPRITE_SCALE;
    float speedPercent = speed / maxSpeed;
    float dx = step * 2.0f * speedPercent; // Velocità laterale massima
    float startPosition = position;

    // Aggiorna le auto
    updateCars(step, playerSegment, playerW);

    // Aggiorna la posizione lungo il tracciato
    position = Util::increase(position, step * speed, trackLength);

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
        speed = Util::accelerate(speed, accel, step);
    }
    else if (keySlower)
    {
        speed = Util::accelerate(speed, breaking, step);
    }
    else
    {
        speed = Util::accelerate(speed, decel, step);
    }

    // Controlla se il giocatore è fuori strada
    if (playerX < -1.0f || playerX > 1.0f)
    {
        if (speed > offRoadLimit)
        {
            speed = Util::accelerate(speed, offRoadDecel, step);
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

            if (lastLapTime <= fastestLapTime || fastestLapTime == 0.0f)
            {
                fastestLapTime = lastLapTime;
                saveScore();
                updateHUD("fast_lap_time", Util::formatTime(lastLapTime));
            }

            updateHUD("last_lap_time", Util::formatTime(lastLapTime));
        }
        else
        {
            currentLapTime += step;
        }
    }
}

float Game::lastY()
{
    if (segments.empty())
        return 0.0f;
    return segments.back().p2.world.y;
}

void Game::updateHUD(std::string key, std::string value)
{
    std::stringstream text;
    if (key == "speed")
    {
        text << value << " Mph";
        DrawTextEx(fontTtf, text.str().c_str(), (Vector2){width - 150.0f, 20.0f}, (float)fontTtf.baseSize, 1, BLACK);
    }
    else if (key == "current_lap_time")
    {
        text << "Time: " << value;
        DrawTextEx(fontTtf, text.str().c_str(), (Vector2){20.0f, 20.0f}, (float)fontTtf.baseSize, 1, BLACK);
    }
    else if (key == "fastest_lap_time")
    {
        text << "Fastest Lap: " << value;
        DrawTextEx(fontTtf, text.str().c_str(), (Vector2){width / 3.0f - 140, 20.0f}, (float)fontTtf.baseSize, 1, BLACK);
    }
    else if (key == "last_lap_time")
        DrawTextEx(fontTtf, value.c_str(), (Vector2){width / 2.0f + 70.0f, 20.0f}, (float)fontTtf.baseSize, 1, BLACK);
}

void Game::renderHUD()
{
    // Draw HUD Rectangle
    DrawRectangle(0, 0, width, 60, Color{0xFF, 0x00, 0x00, 127});
    DrawRectangleLines(0, 0, width, 60, BLACK);

    // Time
    DrawRectangle(10, 10, 150, 40, Color{0xFF, 0xFF, 0xFF, 127});
    DrawRectangleLines(10, 10, 150, 40, BLACK);

    // Speed
    DrawRectangle(width - 160, 10, 150, 40, Color{0xFF, 0xFF, 0xFF, 127});
    DrawRectangleLines(width - 160, 10, 150, 40, BLACK);

    // Current Lap
    DrawRectangle(width / 3 - 150, 10, 250, 40, Color{0xFF, 0xFF, 0xFF, 127});
    DrawRectangleLines(width / 3 - 150, 10, 250, 40, BLACK);

    // Fastest Lap
    DrawRectangle((width / 2) + 70, 10, 250, 40, Color{0xFF, 0xFF, 0xFF, 127});
    DrawRectangleLines((width / 2) + 70, 10, 250, 40, BLACK);

    updateHUD("speed", std::to_string(static_cast<int>(5 * round(speed / 500))));
    updateHUD("current_lap_time", Util::formatTime(currentLapTime));
    updateHUD("fastest_lap_time", Util::formatTime(fastestLapTime));
}

/* Main game functions */
void Game::loadImages()
{
    Image image = LoadImage("resources/images/background.png"); // Loaded in CPU memory (RAM)
    background = LoadTextureFromImage(image);                   // Image converted to texture, GPU memory (VRAM)
    image = LoadImage("resources/images/sprites.png");
    sprites = LoadTextureFromImage(image);
    UnloadImage(image);
}

void Game::pollKeys()
{
    keyLeft = keyRight = keyFaster = keySlower = false;
    if (!paused)
    {
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

        if (IsKeyPressed(KEY_M))
            audio.toggleAudio();

        if (IsKeyPressed(KEY_ONE))
        {
            currentTrack++;
            if (currentTrack == tracks.size())
                currentTrack = 0;
            audio.unloadTrack();
            audio.loadTrack(tracks[currentTrack].c_str());
            audio.playTrack();
        }
    }
    if (IsKeyPressed(KEY_SPACE))
        togglePause();
}

void Game::togglePause()
{
    paused = !paused;
}

void Game::frame()
{
    unsigned int i;
    int n;
    Sprite sprite;
    Car car;
    float spriteScale, spriteX, spriteY;

    Segment &baseSegment = findSegment(position);
    float basePercent = Util::percentRemaining(position, segmentLength);
    Segment &playerSegment = findSegment(position + playerZ);
    float playerPercent = Util::percentRemaining(position + playerZ, segmentLength);
    float playerY = Util::interpolate(playerSegment.p1.world.y, playerSegment.p2.world.y, playerPercent);
    int maxy = height;

    // Rendering
    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawing.DrawBackground(background, width, height, BACKGROUND::SKY, skyOffset, resolution * skySpeed * playerY);
    drawing.DrawBackground(background, width, height, BACKGROUND::HILLS, hillOffset, resolution * hillSpeed * playerY);
    drawing.DrawBackground(background, width, height, BACKGROUND::TREES, treeOffset, resolution * treeSpeed * playerY);

    int x = 0;
    float dx = -(baseSegment.curve * basePercent);

    for (n = 0; n < drawDistance; n++)
    {
        Segment &segment = segments[(baseSegment.index + n) % segments.size()];
        segment.looped = segment.index < baseSegment.index;
        segment.fog = Util::exponentialFog(n / drawDistance, fogDensity);
        segment.clip = maxy;

        Util::project(segment.p1, (playerX * roadWidth) - x, playerY + cameraHeight, position - (segment.looped ? trackLength : 0), cameraDepth, width, height, roadWidth);
        Util::project(segment.p2, (playerX * roadWidth) - x - dx, playerY + cameraHeight, position - (segment.looped ? trackLength : 0), cameraDepth, width, height, roadWidth);

        x = x + dx;
        dx = dx + segment.curve;

        if ((segment.p1.camera.z <= cameraDepth) ||         // behind us
            (segment.p2.screen.y >= segment.p1.screen.y) || // back face cull
            (segment.p2.screen.y >= maxy))                  // clip by (already rendered) hill
            continue;

        drawing.DrawSegment(width, lanes,
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
        Segment &segment = segments[(baseSegment.index + n) % segments.size()];

        for (i = 0; i < segment.cars.size(); i++)
        {
            car = segment.cars[i];
            sprite = car.sprite;
            spriteScale = Util::interpolate(segment.p1.screen.scale, segment.p2.screen.scale, car.percent);
            spriteX = Util::interpolate(segment.p1.screen.x, segment.p2.screen.x, car.percent) + (spriteScale * car.offset * roadWidth * width / 2);
            spriteY = Util::interpolate(segment.p1.screen.y, segment.p2.screen.y, car.percent);
            drawing.DrawSprite(sprites, width, height, resolution, roadWidth, sprite, spriteScale, spriteX, spriteY, -0.5, -1, segment.clip);
        }

        for (i = 0; i < segment.sprites.size(); i++)
        {
            sprite = segment.sprites[i];
            spriteScale = segment.p1.screen.scale;
            spriteX = segment.p1.screen.x + (spriteScale * sprite.offset * roadWidth * width / 2);
            spriteY = segment.p1.screen.y;
            drawing.DrawSprite(sprites, width, height, resolution, roadWidth, sprite, spriteScale, spriteX, spriteY, (sprite.offset < 0 ? -1 : 0), -1, segment.clip);
        }

        if (&segment == &playerSegment)
        {
            drawing.DrawPlayer(sprites, width, height, resolution, roadWidth, speed / maxSpeed,
                               cameraDepth / playerZ,
                               width / 2,
                               (height / 2) - (cameraDepth / playerZ * Util::interpolate(playerSegment.p1.camera.y, playerSegment.p2.camera.y, playerPercent) * height / 2),
                               speed * (keyLeft ? -1 : keyRight ? 1
                                                                : 0),
                               playerSegment.p2.world.y - playerSegment.p1.world.y,
                               paused);
        }
    }

    renderHUD();

    if (paused)
    {
        DrawRectangle(width / 2 - 100, height / 2 - 50, 200, 40, Color{0xFF, 0xFF, 0xFF, 220});
        DrawTextEx(fontTtf, "Game Paused", (Vector2){width / 2 - 90.0f, height / 2 - 40.0f}, (float)fontTtf.baseSize, 1, BLACK);
    }
    DrawFPS(10, height - 30);

    EndDrawing();
}

void Game::addSprite(unsigned int n, Sprite sprite, float offset)
{
    if (n >= 0 && n < segments.size())
    {
        sprite.source = {sprite.x, sprite.y, sprite.w, sprite.h};
        sprite.offset = offset;
        // Aggiungi lo sprite al segmento
        segments[n].sprites.push_back(sprite);
    }
}

void Game::addRoad(int enter, int hold, int leave, float curve, float y)
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
void Game::addStraight(int num = ROAD::LENGTH::MEDIUM)
{
    addRoad(num, num, num, 0.0f, 0.0f);
}

// Funzione per aggiungere una collina
void Game::addHill(int num = ROAD::LENGTH::MEDIUM, int _height = ROAD::HILL::MEDIUM)
{
    addRoad(num, num, num, 0.0f, static_cast<float>(_height));
}

// Funzione per aggiungere una curva
void Game::addCurve(int num = ROAD::LENGTH::MEDIUM, int curve = ROAD::CURVE::MEDIUM, int _height = ROAD::HILL::NONE)
{
    addRoad(num, num, num, static_cast<float>(curve), static_cast<float>(_height));
}

// Funzione per aggiungere colline basse ondulate
void Game::addLowRollingHills(int num = ROAD::LENGTH::SHORT, int _height = ROAD::HILL::LOW)
{
    addRoad(num, num, num, 0.0f, static_cast<float>(_height) / 2.0f);
    addRoad(num, num, num, 0.0f, -static_cast<float>(_height));
    addRoad(num, num, num, static_cast<float>(ROAD::CURVE::EASY), static_cast<float>(_height));
    addRoad(num, num, num, 0.0f, 0.0f);
    addRoad(num, num, num, -static_cast<float>(ROAD::CURVE::EASY), static_cast<float>(_height) / 2.0f);
    addRoad(num, num, num, 0.0f, 0.0f);
}

// Funzione per aggiungere curve a forma di S
void Game::addSCurves()
{
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::EASY, ROAD::HILL::NONE);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::CURVE::MEDIUM, ROAD::HILL::MEDIUM);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::CURVE::EASY, -ROAD::HILL::LOW);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::EASY, ROAD::HILL::MEDIUM);
    addRoad(ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, ROAD::LENGTH::MEDIUM, -ROAD::CURVE::MEDIUM, -ROAD::HILL::MEDIUM);
}

// Funzione per aggiungere dossi
void Game::addBumps()
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
void Game::addDownhillToEnd(int num = 200)
{
    addRoad(num, num, num, -ROAD::CURVE::EASY, -lastY() / segmentLength);
}

// Funzione per aggiornare la posizione delle auto
void Game::updateCars(float dt, Segment &playerSegment, float playerW)
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

        // printf("Car %d: %f - %f - %f\n", car.index, car.z, car.offset, car.percent);
    }
}

float Game::updateCarOffset(Car &car, Segment &carSegment, Segment &playerSegment, float playerW)
{
    const int lookahead = 20;                 // Distanza di previsione
    float carW = car.sprite.w * SPRITE_SCALE; // Larghezza dell'auto

    // Ottimizzazione: ignora le auto fuori dalla vista del giocatore
    if ((carSegment.index - playerSegment.index) > drawDistance)
    {
        return 0.0f;
    }

    for (int i = 1; i < lookahead; i++)
    {
        const Segment &segment = segments[(carSegment.index + i) % segments.size()];

        // Controllo collisione con il giocatore
        if (&segment == &playerSegment && car.speed > speed && Util::overlap(playerX, playerW, car.offset, carW, 1.2f))
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
            float otherCarW = otherCar.sprite.w * SPRITE_SCALE;
            if (car.speed > otherCar.speed && Util::overlap(car.offset, carW, otherCar.offset, otherCarW, 1.2f))
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

void Game::resetSprites()
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
        addSprite(n, SPRITES::PALM_TREE, 0.5f + Util::randomFloat() * 0.5f);
        addSprite(n, SPRITES::PALM_TREE, 1.0f + Util::randomFloat() * 2.0f);
    }

    // Aggiungi colonne e alberi
    for (int n = 250; n < 1000; n += 5)
    {
        addSprite(n, SPRITES::COLUMN, 1.1f);
        addSprite(n + Util::randomInt(0, 5), SPRITES::TREE1, -1.0f - Util::randomFloat() * 2.0f);
        addSprite(n + Util::randomInt(0, 5), SPRITES::TREE2, -1.0f - Util::randomFloat() * 2.0f);
    }

    // Aggiungi piante
    for (unsigned int n = 200; n < segments.size(); n += 3)
    {
        addSprite(n, Util::randomChoice(PLANTS), Util::randomChoice(choices) * (2.0f + Util::randomFloat() * 5.0f));
    }

    // Aggiungi sprite complessi
    for (int n = 1000; n < static_cast<int>(segments.size()) - 50; n += 100)
    {
        float side = Util::randomChoice(choices);
        addSprite(n + Util::randomInt(0, 50), Util::randomChoice(BILLBOARDS), -side);

        for (int i = 0; i < 20; ++i)
        {
            Sprite sprite = Util::randomChoice(PLANTS);
            float offset = side * (1.5f + Util::randomFloat());
            addSprite(n + Util::randomInt(0, 50), sprite, offset);
        }
    }
}

void Game::resetRoad()
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

void Game::resetCars()
{
    cars.clear();
    float _speed;

    for (int n = 0; n < totalCars; n++)
    {
        // Calcola l'offset casuale e scegli un lato casuale
        float offset = Util::randomFloat() * Util::randomChoice(std::vector<float>{-0.8f, 0.8f});

        // Calcola la posizione z casuale
        int z = Util::randomFloat() * static_cast<float>(segments.size()) * segmentLength;

        // Seleziona uno sprite casuale
        Sprite sprite = Util::randomChoice(CARS);

        float c = 2.0f;
        if (sprite.h == SPRITES::SEMI.h &&
            sprite.y == SPRITES::SEMI.y &&
            sprite.x == SPRITES::SEMI.x &&
            sprite.w == SPRITES::SEMI.w)
            c = 4.0f;
        // Calcola la velocità dell'auto
        _speed = maxSpeed / 4.0f + Util::randomFloat() * maxSpeed / c;

        // Crea l'auto
        Car car = {n, offset, z, sprite, _speed, 0.0f};

        // Trova il segmento corrispondente e aggiungi l'auto
        Segment &segment = findSegment(car.z);
        segment.cars.push_back(car);

        // Aggiungi l'auto alla lista globale
        cars.push_back(car);
    }
}

void Game::loadOptions(std::map<std::string, std::string> options)
{
    // Leggi o usa i valori di default
    width = options.count("width") ? Util::toInt(options["width"], 1024) : 1024;
    height = options.count("height") ? Util::toInt(options["height"], 768) : 768;
    lanes = options.count("lanes") ? Util::toInt(options["lanes"], 3) : 3;
    roadWidth = options.count("roadWidth") ? Util::toFloat(options["roadWidth"], 2000.0f) : 2000.0f;
    cameraHeight = options.count("cameraHeight") ? Util::toFloat(options["cameraHeight"], 1000.0f) : 1000.0f;
    drawDistance = options.count("drawDistance") ? Util::toInt(options["drawDistance"], 300) : 300;
    fogDensity = options.count("fogDensity") ? Util::toFloat(options["fogDensity"], 5.0f) : 5.0f;
    fieldOfView = options.count("fieldOfView") ? Util::toFloat(options["fieldOfView"], 100.0f) : 100.0f;
    segmentLength = options.count("segmentLength") ? Util::toFloat(options["segmentLength"], 200.0f) : 200.0f;
    rumbleLength = options.count("rumbleLength") ? Util::toInt(options["rumbleLength"], 3) : 3;

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

/* Segments functions */
void Game::addSegment(float curve, float y)
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

Segment &Game::findSegment(float z)
{
    int index = static_cast<int>(std::floor(z / segmentLength)) % segments.size();
    return segments[index];
}
