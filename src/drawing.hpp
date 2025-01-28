#ifndef __DRAWING_HPP__
#define __DRAWING_HPP__

#include "raylib.h"
#include "util.hpp"

class Drawing {
    public:
        // Funzione per disegnare un poligono
        void DrawPolygon(Color color, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
        // Funzione per disegnare un segmento di strada
        void DrawSegment(int screenWidth, int lanes, float x1, float y1, float w1, float x2, float y2, float w2, float fog, const Colors& color);
        // Funzione per disegnare un elemento di sfondo
        void DrawBackground(const Texture2D& background, int _width, int _height, const Sprite& layer, float rotation, float offset);
        // Funzione per disegnare uno sprite
        void DrawSprite(const Texture2D& spriteSheet, int screenWidth, int screenHeight, float resolution, float roadWidth,
                        const Sprite& sprite, float scale, float destX, float destY, float offsetX, float offsetY, float clipY);
        // Funzione per disegnare la nebbia
        void DrawFog(int x, int y, int _width, int _height, float fogIntensity);
        void DrawPlayer(Texture2D texture, int _width, int _height, float resolution, float roadWidth, float speedPercent, float scale, float destX, float destY, float steer, float updown, bool paused);
};

#endif