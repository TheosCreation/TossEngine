#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class TOSSENGINE_API Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const glm::vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}

    float Length() const { return glm::length(static_cast<glm::vec3>(*this)); }
    Vector3 Normalized() const
    {
        glm::vec3 v = static_cast<glm::vec3>(*this);
        v = glm::normalize(v);
        return Vector3(v);
    }

    Mat4 Translate(const Mat4& matrix) const {
        return glm::translate(matrix, static_cast<glm::vec3>(*this));
    }
    Mat4 Scale(const Mat4& matrix) const {
        return glm::scale(matrix, static_cast<glm::vec3>(*this));
    }


    operator glm::vec3() const { return glm::vec3(x, y, z); }

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }

    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3& operator*=(float scalar) { x *= scalar, y *= scalar, z *= scalar; return *this; }

    Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
    Vector3& operator*=(const Vector3& other) { x *= other.x, y *= other.y, z *= other.z; return *this; }

    Vector3 operator*(const Quaternion& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
};
