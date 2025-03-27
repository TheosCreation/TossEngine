#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class TOSSENGINE_API Vector2 {
public:
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float val) : x(val), y(val) {}
    Vector2(float x, float y) : x(x), y(y) {}
    Vector2(const glm::vec2& v) : x(v.x), y(v.y) {}

    operator glm::vec2() const { return glm::vec2(x, y); }
};
