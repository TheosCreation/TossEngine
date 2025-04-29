#pragma once
#include <random>
#include "Vector2.h"
#include "Vector3.h"

/**
 * @class Random
 * @brief Utility class for generating random numbers and vectors.
 */
class TOSSENGINE_API Random
{
public:
    // Initialize internal RNG
    static void Init();

    // Random integer [min, max] inclusive
    static int Range(int min, int max);

    // Random float [min, max]
    static float Range(float min, float max);

    // Random float between 0.0 and 1.0
    static float Value();

    // Random 3D vector inside unit sphere
    static Vector3 InsideUnitSphere();

    // Random 2D vector inside unit circle
    static Vector2 InsideUnitCircle();

    // Random boolean
    static bool Bool();

private:
    static std::mt19937 s_randomEngine;
};
