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

    float* Data() { return &x; }
    const float* Data() const {
        return &x;
    }

    float Length() const { return glm::length(static_cast<glm::vec2>(*this)); }
    Vector2 Normalized() const
    {
        glm::vec2 v = static_cast<glm::vec2>(*this);
        v = glm::normalize(v);
        return Vector2(v);
    }
    
    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ")";
        return oss.str();
    }

    operator glm::vec2() const { return glm::vec2(x, y); }
    
    Vector2 operator-() const {
        return Vector2(-x, -y);
    }
    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }


    bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2& other) const {
        return x != other.x || y != other.y;
    }
};

inline void to_json(json& j, Vector2 const& v) {
    j = json{
        { "x", v.x },
        { "y", v.y }
    };
}

inline void from_json(json const& j, Vector2& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
}