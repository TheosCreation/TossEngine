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
