#include "Random.h"
#include <ctime>
#include <gtc/constants.hpp>

std::mt19937 Random::s_randomEngine;

void Random::Init()
{
    std::random_device rd;
    s_randomEngine.seed(rd());
}

int Random::Range(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(s_randomEngine);
}

float Random::Range(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_randomEngine);
}

float Random::Value()
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(s_randomEngine);
}

Vector3 Random::InsideUnitSphere()
{
    while (true)
    {
        Vector3 p = Vector3(
            Range(-1.0f, 1.0f),
            Range(-1.0f, 1.0f),
            Range(-1.0f, 1.0f)
        );
        if (p.Length() <= 1.0f)
            return p;
    }
}

Vector2 Random::InsideUnitCircle()
{
    while (true)
    {
        Vector2 p = Vector2(
            Range(-1.0f, 1.0f),
            Range(-1.0f, 1.0f)
        );
        if (p.Length() <= 1.0f)
            return p;
    }
}

bool Random::Bool()
{
    std::bernoulli_distribution dist(0.5);
    return dist(s_randomEngine);
}