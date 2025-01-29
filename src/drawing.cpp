#include "raylib.h"

#include "common.hpp"
#include "drawing.hpp"

#include <cstdlib>

// Funzione per disegnare un poligono
void Drawing::DrawPolygon(Color color, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    Vector2 vertices[] = {{x1, y1},
                          {x2, y2},
                          {x3, y3},
                          {x4, y4}};
    DrawTriangle(vertices[0], vertices[1], vertices[2], color);
    DrawTriangle(vertices[2], vertices[3], vertices[0], color);
}

// Funzione per disegnare un segmento di strada
void
Drawing::DrawSegment(int screenWidth, int _lanes, float x1, float y1, float w1, float x2, float y2, float w2, float fog,
                     const Colors &color) {
    float r1 = w1 / std::max(6.0f, 2.0f * _lanes);
    float r2 = w2 / std::max(6.0f, 2.0f * _lanes);
    float l1 = w1 / std::max(32.0f, 8.0f * _lanes);
    float l2 = w2 / std::max(32.0f, 8.0f * _lanes);

    // Disegna l'erba
    DrawRectangle(0, y2, screenWidth, y1 - y2, color.grass);

    // Disegna il bordo (rumble strips)
    DrawPolygon(color.rumble, x1 - w1 - r1, y1, x1 - w1, y1, x2 - w2, y2, x2 - w2 - r2, y2);
    DrawPolygon(color.rumble, x1 + w1 + r1, y1, x1 + w1, y1, x2 + w2, y2, x2 + w2 + r2, y2);

    // Disegna la strada
    DrawPolygon(color.road, x1 - w1, y1, x1 + w1, y1, x2 + w2, y2, x2 - w2, y2);

    // Disegna le linee della corsia
    if (color.lane.a > 0) { // Se è specificato un colore per le linee
        float laneWidth1 = w1 * 2 / _lanes;
        float laneWidth2 = w2 * 2 / _lanes;
        float laneX1 = x1 - w1 + laneWidth1;
        float laneX2 = x2 - w2 + laneWidth2;

        for (int lane = 1; lane < _lanes; lane++) {
            DrawPolygon(color.lane, laneX1 - l1 / 2, y1, laneX1 + l1 / 2, y1, laneX2 + l2 / 2, y2, laneX2 - l2 / 2, y2);
            laneX1 += laneWidth1;
            laneX2 += laneWidth2;
        }
    }
}

// Funzione per disegnare un elemento di sfondo
void Drawing::DrawBackground(const Texture2D &_background, int _width, int _height, const Sprite &layer,
                             float rotation = 0.0f, float offset = 0.0f) {
    int imageW = layer.w / 2;
    int imageH = layer.h;

    int sourceX = layer.x + static_cast<int>(layer.w * rotation) % layer.w;
    int sourceY = layer.y;
    int sourceW = std::min(imageW, layer.w - sourceX);
    int sourceH = imageH;

    int destX = 0;
    int destY = offset;
    int destW = static_cast<int>(_width * (static_cast<float>(sourceW) / imageW));
    int destH = _height;

    Rectangle sourceRec = {static_cast<float>(sourceX), static_cast<float>(sourceY), static_cast<float>(sourceW),
                           static_cast<float>(sourceH)};
    Rectangle destRec = {static_cast<float>(destX), static_cast<float>(destY), static_cast<float>(destW),
                         static_cast<float>(destH)};

    DrawTexturePro(_background, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    if (sourceW < imageW) {
        sourceRec = {static_cast<float>(layer.x), static_cast<float>(sourceY), static_cast<float>(imageW - sourceW),
                     static_cast<float>(sourceH)};
        destRec = {static_cast<float>(destW - 1), static_cast<float>(destY), static_cast<float>(_width - destW),
                   static_cast<float>(destH)};
        DrawTexturePro(_background, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    }
}

// Funzione per disegnare uno sprite
void Drawing::DrawSprite(const Texture2D &spriteSheet, int screenWidth, int screenHeight, float _resolution,
                         float _roadWidth,
                         const Sprite &sprite, float scale, float destX, float destY, float offsetX = 0.0f,
                         float offsetY = 0.0f, float clipY = 0.0f) {
    float destW = (sprite.w * scale * screenWidth / 2) * SPRITE_SCALE * _roadWidth;
    float destH = (sprite.h * scale * screenWidth / 2) * SPRITE_SCALE * _roadWidth;

    destX += destW * offsetX;
    destY += destH * offsetY;

    float clipH = clipY > 0.0f ? std::max(0.0f, destY + destH - clipY) : 0.0f;
    if (clipH < destH) {
        Rectangle sourceRec = {static_cast<float>(sprite.x), static_cast<float>(sprite.y), static_cast<float>(sprite.w),
                               static_cast<float>(sprite.h) - (sprite.h * clipH / destH)};
        Rectangle destRec = {destX, destY, destW, destH - clipH};
        DrawTexturePro(spriteSheet, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    }
}

// Funzione per disegnare la nebbia
void Drawing::DrawFog(int x, int y, int _width, int _height, float fogIntensity) {
    if (fogIntensity < 1.0f) {
        Color fogColor = {FOG.road.r, FOG.road.g, FOG.road.b, static_cast<unsigned char>(255 * (1.0f - fogIntensity))};
        DrawRectangle(x, y, _width, _height, fogColor);
    }
}

void
Drawing::DrawPlayer(Texture2D texture, int _width, int _height, float _resolution, float _roadWidth, float speedPercent,
                    float scale, float destX, float destY, float steer, float updown, bool paused) {
    float bounce = (1.5f * Util::randomFloat() * speedPercent * _resolution) * ((rand() % 2) == 0 ? -1 : 1);
    if (paused)
        bounce = 0.0f;

    Sprite sprite;

    if (steer < 0)
        sprite = (updown > 0) ? SPRITES::PLAYER_UPHILL_LEFT : SPRITES::PLAYER_LEFT;
    else if (steer > 0)
        sprite = (updown > 0) ? SPRITES::PLAYER_UPHILL_RIGHT : SPRITES::PLAYER_RIGHT;
    else
        sprite = (updown > 0) ? SPRITES::PLAYER_UPHILL_STRAIGHT : SPRITES::PLAYER_STRAIGHT;

    DrawSprite(texture, _width, _height, _resolution, _roadWidth, sprite, scale, destX, destY + bounce, -0.5f, -1.0f);
}

