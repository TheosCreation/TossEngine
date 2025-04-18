#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class Vector3;

class TOSSENGINE_API Vector4 {
public:
    float x, y, z, w;

    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float val) : x(val), y(val), z(val), w(val) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    constexpr Vector4(const Vector3& v, float w);


    float* Data() { return &x; }
    const float* Data() const {
        return &x;
    }

    operator glm::vec4() const { return glm::vec4(x, y, z, w); }
    
    Vector4 operator-() const {
        return Vector4(-x, -y, -z, -w);
    }
    Vector4& operator-=(const Vector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }

    bool operator==(const Vector4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const Vector4& other) const {
        return x != other.x || y != other.y || z != other.z || w != other.w;
    }

};

inline void to_json(json& j, Vector4 const& v) {
    j = json{
        { "x", v.x },
        { "y", v.y },
        { "z", v.z },
        { "w", v.w },
    };
}

inline void from_json(json const& j, Vector4& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
    if (j.contains("z") && !j["z"].is_null()) j.at("z").get_to(v.z);
    if (j.contains("w") && !j["w"].is_null()) j.at("w").get_to(v.w);
}