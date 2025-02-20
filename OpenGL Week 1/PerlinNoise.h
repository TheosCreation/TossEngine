#pragma once
#include "Math.h"

class PerlinNoise
{
public:
    PerlinNoise();
    ~PerlinNoise() = default;
     /**
     * @brief Generates a pseudo-random value based on Perlin noise algorithm.
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     * @param seed The seed value for random generation.
     * @return A pseudo-random value between 0 and 1.
     */
    double RandomValue(int x, int y);
    
    /**
     * @brief Smooths the noise by averaging the values of neighboring points.
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     * @return A smoothed value based on the surrounding points.
     */
    double Smooth(int x, int y);
    
    /**
     * @brief Performs linear interpolation between two points.
     * @param Point1 The first point.
     * @param Point2 The second point.
     * @param Fract The fractional value for interpolation.
     * @return The interpolated value.
     */
    double LinearInterpolate(double Point1, double Point2, double Fract);

    /**
     * @brief Performs cosine interpolation between two points.
     * @param Point1 The first point.
     * @param Point2 The second point.
     * @param Fract The fractional value for interpolation.
     * @return The interpolated value.
     */
    double CosineInterpolate(double Point1, double Point2, double Fract);

    double SmoothInterpolate(double X, double Y);

    /**
     * @brief Generates the total Perlin noise value for a given point (X, Y).
     * @param X The x-coordinate.
     * @param Y The y-coordinate.
     * @return The total Perlin noise value normalized between 0 and 1.
     */
    double TotalNoisePerPoint(int X, int Y);

    int Seed = 0;
private:
};

