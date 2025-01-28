#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>
#include <string>

#include "common.hpp"

class Util
{
public:
    // Restituisce il timestamp corrente in millisecondi
    static long long timestamp()
    {
        return static_cast<long long>(std::time(nullptr)) * 1000;
    }

    static float randomFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // Converte un valore in intero con un valore di default
    static int toInt(const std::string &obj, int def)
    {
        try
        {
            return std::stoi(obj);
        }
        catch (...)
        {
            return def;
        }
    }

    // Converte un valore in float con un valore di default
    static float toFloat(const std::string &obj, float def)
    {
        try
        {
            return std::stof(obj);
        }
        catch (...)
        {
            return def;
        }
    }

    // Limita un valore tra un minimo e un massimo
    static float limit(float value, float min, float max)
    {
        return std::max(min, std::min(value, max));
    }

    // Restituisce un intero casuale tra min e max
    static int randomInt(int min, int max)
    {
        return min + rand() % ((max - min) + 1);
    }

    // Restituisce una scelta casuale da un array
    template <typename T>
    static T randomChoice(const std::vector<T> &options)
    {
        return options[randomInt(0, options.size() - 1)];
    }

    // Calcola la percentuale rimanente di n rispetto a total
    static float percentRemaining(float n, float total)
    {
        return fmod(n, total) / total;
    }

    // Aumenta una velocità con accelerazione e delta tempo
    static float accelerate(float v, float _accel, float dt)
    {
        return v + (_accel * dt);
    }

    // Interpolazione lineare
    static float interpolate(float a, float b, float percent)
    {
        return a + (b - a) * percent;
    }

    // Easing in
    static float easeIn(float a, float b, float percent)
    {
        return a + (b - a) * std::pow(percent, 2);
    }

    // Easing out
    static float easeOut(float a, float b, float percent)
    {
        return a + (b - a) * (1 - std::pow(1 - percent, 2));
    }

    // Easing in-out
    static float easeInOut(float a, float b, float percent)
    {
        return a + (b - a) * ((-std::cos(percent * M_PI) / 2) + 0.5);
    }

    // Nebbia esponenziale
    static float exponentialFog(float distance, float density)
    {
        return 1.0f / std::exp(distance * distance * density);
    }

    // Incremento ciclico
    static float increase(float start, float increment, float max)
    {
        float result = start + increment;
        while (result >= max)
            result -= max;
        while (result < 0)
            result += max;
        return result;
    }

    // Proiezione prospettica
    static void project(Point3D &p, float cameraX, float cameraY, float cameraZ, float _cameraDepth, float _width, float _height, float _roadWidth)
    {
        p.camera.x = p.world.x - cameraX;
        p.camera.y = p.world.y - cameraY;
        p.camera.z = p.world.z - cameraZ;
        p.screen.scale = _cameraDepth / p.camera.z;
        p.screen.x = round((_width / 2) + (p.screen.scale * p.camera.x * _width / 2));
        p.screen.y = round((_height / 2) - (p.screen.scale * p.camera.y * _height / 2));
        p.screen.w = round(p.screen.scale * _roadWidth * _width / 2);
    }

    static bool overlap(Rectangle player, Rectangle car) {
        return CheckCollisionRecs(player, car);
    }

    // Controlla se due oggetti si sovrappongono
    static bool overlap(float x1, float w1, float x2, float w2, float percent = 1.0f)
    {
        float half = percent / 2;
        float min1 = x1 - (w1 * half);
        float max1 = x1 + (w1 * half);
        float min2 = x2 - (w2 * half);
        float max2 = x2 + (w2 * half);
        return !(max1 < min2 || min1 > max2);
    }

    static std::string formatTime(float dt)
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
};

#endif