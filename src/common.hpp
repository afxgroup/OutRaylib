#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <vector>
#include <map>
#include <string>
#include "raylib.h"

typedef struct Source {
    int x;  // Coordinata X
    int y;  // Coordinata Y
    int w;  // Larghezza
    int h;  // Altezza
} Source;

typedef struct Sprite {
    int x;  // Coordinata X
    int y;  // Coordinata Y
    int w;  // Larghezza
    int h;  // Altezza

    float offset;
    Source source;
} Sprite;

class Car {
public:
    int index;
    float offset; // Offset dell'auto sulla strada (-1 a 1)
    float z;
    Sprite sprite; // Sprite dell'auto
    float speed;  // Velocità dell'auto
    float percent;
    bool operator==(const Car &car) const {
        return car.index == index;
    }
};

struct Colors {
    Color road;   // Colore della strada
    Color grass;  // Colore dell'erba
    Color rumble; // Colore delle strisce rumble
    Color lane;   // Colore delle linee della corsia (opzionale)
};

const Colors SKY = {0x72, 0xD7, 0xEE, 0xFF};
const Colors TREE = {0x00, 0x51, 0x08, 0xFF};
const Colors FOG = {0x00, 0x51, 0x08, 0xFF};

const Colors LIGHT = {
    {0x6B, 0x6B, 0x6B, 0xff}, // Road (#6B6B6B)
    {0x10, 0xAA, 0x10, 0xff}, // Grass (#10AA10)
    {0x55, 0x55, 0x55, 0xff}, // Rumble (#555555)
    {0xCC, 0xCC, 0xCC, 0xFF}  // Lane (#CCCCCC)
};

const Colors DARK = {
    { 0x69, 0x69, 0x69, 0xff },
    { 0x00, 0x9A, 0x00, 0xff },
    { 0xBB, 0xBB, 0xBB, 0xff }
};

const Colors START = {
    {0xff, 0xff, 0xff, 0xff}, // Road (White)
    {0xff, 0xff, 0xff, 0xff}, // Grass (White)
    {0xff, 0xff, 0xff, 0xff}  // Rumble (White)
};

const Colors FINISH = {
    {0, 0, 0, 0xFF},       // Road (Black)
    {0, 0, 0, 0xFF},       // Grass (Black)
    {0, 0, 0, 0xFF}        // Rumble (Black)
};

// Punto 3D (mondo, camera, schermo)
struct Point3D {
    struct {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    } world;

    struct {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    } camera;

    struct {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float scale = 0.0f;
    } screen;
};

struct Segment {
    int index;                      // Indice del segmento
    std::vector<Car> cars;          // Auto nel segmento
    Point3D p1, p2;                 // Punti del segmento
    float curve;                    // Curva del segmento
    std::vector<Sprite> sprites;    // Indici degli sprite (da implementare)
    Colors color;                   // Colore del segmento
    bool looped;
    float fog;
    float clip;
};

struct ROAD {
    struct LENGTH {
        static constexpr int NONE = 0;
        static constexpr int SHORT = 25;
        static constexpr int MEDIUM = 50;
        static constexpr int LONG = 100;
    };

    struct HILL {
        static constexpr int NONE = 0;
        static constexpr int LOW = 20;
        static constexpr int MEDIUM = 40;
        static constexpr int HIGH = 60;
    };

    struct CURVE {
        static constexpr int NONE = 0;
        static constexpr int EASY = 2;
        static constexpr int MEDIUM = 4;
        static constexpr int HARD = 6;
    };
};

// Namespace per sfondi
namespace BACKGROUND {
    const Sprite HILLS = {5, 5, 1280, 480};
    const Sprite SKY = {5, 495, 1280, 480};
    const Sprite TREES = {5, 985, 1280, 480};
}

// Namespace per sprite
namespace SPRITES {
    const Sprite PALM_TREE = {5, 5, 215, 540, -1 };
    const Sprite BILLBOARD08 = {230, 5, 385, 265, -1 };
    const Sprite TREE1 = {625, 5, 360, 360, -1 };
    const Sprite DEAD_TREE1 = {5, 555, 135, 332, -1 };
    const Sprite BILLBOARD09 = {150, 555, 328, 282, -1 };
    const Sprite BOULDER3 = {230, 280, 320, 220, -1 };
    const Sprite COLUMN = {995, 5, 200, 315, -1 };
    const Sprite BILLBOARD01 = {625, 375, 300, 170, -1 };
    const Sprite BILLBOARD06 = {488, 555, 298, 190, -1 };
    const Sprite BILLBOARD05 = {5, 897, 298, 190, -1 };
    const Sprite BILLBOARD07 = {313, 897, 298, 190, -1 };
    const Sprite BOULDER2 = {621, 897, 298, 140, -1 };
    const Sprite TREE2 = {1205, 5, 282, 295, -1 };
    const Sprite BILLBOARD04 = {1205, 310, 268, 170, -1 };
    const Sprite DEAD_TREE2 = {1205, 490, 150, 260, -1 };
    const Sprite BOULDER1 = {1205, 760, 168, 248, -1 };
    const Sprite BUSH1 = {5, 1097, 240, 155, -1 };
    const Sprite CACTUS = {929, 897, 235, 118, -1 };
    const Sprite BUSH2 = {255, 1097, 232, 152, -1 };
    const Sprite BILLBOARD03 = {5, 1262, 230, 220, -1 };
    const Sprite BILLBOARD02 = {245, 1262, 215, 220, -1 };
    const Sprite STUMP = {995, 330, 195, 140, -1 };
    const Sprite SEMI = {1365, 490, 122, 144, -1 };
    const Sprite TRUCK = {1365, 644, 100, 78, -1 };
    const Sprite CAR03 = {1383, 760, 88, 55, -1 };
    const Sprite CAR02 = {1383, 825, 80, 59, -1 };
    const Sprite CAR04 = {1383, 894, 80, 57, -1 };
    const Sprite CAR01 = {1205, 1018, 80, 56, -1 };
    const Sprite PLAYER_UPHILL_LEFT = {1383, 961, 80, 45, -1 };
    const Sprite PLAYER_UPHILL_STRAIGHT = {1295, 1018, 80, 45, -1 };
    const Sprite PLAYER_UPHILL_RIGHT = {1385, 1018, 80, 45, -1 };
    const Sprite PLAYER_LEFT = {995, 480, 80, 41, -1 };
    const Sprite PLAYER_STRAIGHT = {1085, 480, 80, 41, -1 };
    const Sprite PLAYER_RIGHT = {995, 531, 80, 41, -1 };
}

// Costante per il calcolo della scala degli sprite
const float SPRITE_SCALE = 0.3f * (1.0f / static_cast<float>(SPRITES::PLAYER_STRAIGHT.w)); // La larghezza di riferimento è 1/3 della (metà) larghezza della strada

// Liste di sprite
const std::vector<Sprite> BILLBOARDS = {
    SPRITES::BILLBOARD01, SPRITES::BILLBOARD02, SPRITES::BILLBOARD03,
    SPRITES::BILLBOARD04, SPRITES::BILLBOARD05, SPRITES::BILLBOARD06,
    SPRITES::BILLBOARD07, SPRITES::BILLBOARD08, SPRITES::BILLBOARD09
};

const std::vector<Sprite> PLANTS = {
    SPRITES::TREE1, SPRITES::TREE2, SPRITES::DEAD_TREE1, SPRITES::DEAD_TREE2,
    SPRITES::PALM_TREE, SPRITES::BUSH1, SPRITES::BUSH2, SPRITES::CACTUS,
    SPRITES::STUMP, SPRITES::BOULDER1, SPRITES::BOULDER2, SPRITES::BOULDER3
};

const std::vector<Sprite> CARS = {
    SPRITES::CAR01, SPRITES::CAR02, SPRITES::CAR03, SPRITES::CAR04,
    SPRITES::SEMI, SPRITES::TRUCK
};

std::string formatTime(float dt);
void reset(std::map<std::string, int> options);
bool overlap(float x1, float w1, float x2, float w2, float percent);
float updateCarOffset(Car& car, Segment& carSegment, Segment& playerSegment, float playerW /*, const std::vector<Segment>& segments*/);
Segment& findSegment(float z);
void addSegment(float curve, float y);
float lastY();
void addSprite(int n, Sprite source, float offset);
void addRoad(int enter, int hold, int leave, float curve, float y);
void resetRoad();
void addStraight(int num);
void addHill(int num, int height);
void addCurve(int num, int curve, int height);
void addLowRollingHills(int num, int height);
void addSCurves();
void addBumps();
void addDownhillToEnd(int num);
void resetSprites();
void resetCars();
void updateCars(float dt, Segment& playerSegment, float playerW);

#endif