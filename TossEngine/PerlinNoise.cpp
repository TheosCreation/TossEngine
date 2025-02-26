#include "PerlinNoise.h"
#include <time.h>

PerlinNoise::PerlinNoise()
{
    Seed = (int)time(NULL);
}

double PerlinNoise::RandomValue(int x, int y)
{
    int noise = x + y * Seed;
    noise = (noise << 13) ^ noise;
    int t = (noise * (noise * noise * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - (double)t * 0.93132257461548515625e-9;
}

double PerlinNoise::Smooth(int x, int y)
{
    double Corners = (RandomValue(x - 1, y - 1) + RandomValue(x + 1, y - 1) +
        RandomValue(x - 1, y + 1) + RandomValue(x + 1, y + 1)) / 16.0f;

    double Sides = (RandomValue(x - 1, y) + RandomValue(x + 1, y) +
        RandomValue(x, y - 1) + RandomValue(x, y + 1)) / 8.0f;

    double Center = RandomValue(x, y) / 4.0f;

    // Return the sum of the contributions
    return Corners + Sides + Center;
}

double PerlinNoise::LinearInterpolate(double Point1, double Point2, double Fract)
{
    return (Point1 * (1 - Fract) + Point2 * Fract);
}

double PerlinNoise::CosineInterpolate(double Point1, double Point2, double Fract)
{
    double Fract2 = (1 - cos(Fract * PI)) / 2;
    return (Point1 * (1 - Fract2) + Point2 * Fract2);
}

double PerlinNoise::SmoothInterpolate(double X, double Y)
{
    // Truncate the X and Y coordinates 
    int TruncatedX = (int)X;
    int TruncatedY = (int)Y;

    // Get the fractional component of X and Y (only the value after the decimal)
    double FractX = X - (double)TruncatedX;
    double FractY = Y - (double)TruncatedY;

    // Smoothing
    double V1 = Smooth(TruncatedX, TruncatedY);
    double V2 = Smooth(TruncatedX + 1, TruncatedY);
    double V3 = Smooth(TruncatedX, TruncatedY + 1);
    double V4 = Smooth(TruncatedX + 1, TruncatedY + 1);

    // Interpolates
    double Interpolate_1 = CosineInterpolate(V1, V2, FractX);
    double Interpolate_2 = CosineInterpolate(V3, V4, FractX);

    // Final interpolation
    double Final = CosineInterpolate(Interpolate_1, Interpolate_2, FractY);

    return Final;
}


double PerlinNoise::TotalNoisePerPoint(int X, int Y)
{
    int Octaves = 4;
    float Wavelength = 128.0f;
    float Gain = 0.5f;
    float Lacunarity = 2.0f;

    float MaxValue = 0.0f;
    double Total = 0.0f;

    for (int i = 0; i < Octaves; i++)
    {
        float Frequency = (float)(pow(Lacunarity, i)) / Wavelength;
        float Amplitude = (float)(pow(Gain, i));
        MaxValue += Amplitude;

        Total += SmoothInterpolate(X * Frequency, Y * Frequency) * Amplitude;
    }

    return (Total / MaxValue);
}