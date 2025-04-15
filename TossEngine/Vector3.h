#pragma once
#include "TossEngineAPI.h"
#include "Math.h"
#include "reactphysics3d\mathematics\Vector3.h"

class Mat4;
class Quaternion;
class Vector2;
class Vector4;

class TOSSENGINE_API Vector3 {
public:
    float x, y, z;

    constexpr Vector3() : x(0), y(0), z(0) {}
    constexpr Vector3(float val) : x(val), y(val), z(val) {}
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const glm::vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}
    Vector3(const reactphysics3d::Vector3& vec) : x(vec.x), y(vec.y), z(vec.z) {}
    Vector3(const Vector2& vec);

    float* Data() { return &x; }
    const float* Data() const {
        return &x;
    }

    float Length() const { return glm::length(static_cast<glm::vec3>(*this)); }

    /**
     * Creates a normalized copy of the vector
     * @return Vector3: A normalized vector
     */
    Vector3 Normalized() const {
        return Vector3(glm::normalize(static_cast<glm::vec3>(*this)));
    }

    static Vector3 Cross(const Vector3& a, const Vector3& b) {
        return Vector3(glm::cross(static_cast<glm::vec3>(a), static_cast<glm::vec3>(b)));
    }

    static float Distance(const Vector3& a, const Vector3& b) {
        return glm::distance(static_cast<glm::vec3>(a), static_cast<glm::vec3>(b));
    }

    static Vector3 ExtractTranslation(const Mat4& m);
    static Vector3 ExtractScale(const Mat4& m);

    Mat4 ToTranslation() const;
    Mat4 ToScale() const; 

    Vector3 ToDegrees() const {
        return Vector3(glm::degrees(static_cast<glm::vec3>(*this)));
    }

    Vector3 ToRadians() const {
        return Vector3(glm::radians(static_cast<glm::vec3>(*this)));
    }

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    operator glm::vec3() const { return glm::vec3(x, y, z); }

    operator reactphysics3d::Vector3() const { return reactphysics3d::Vector3(x, y, z); }

    operator std::string() const {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z;return *this; }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }

    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3& operator*=(float scalar) { x *= scalar, y *= scalar, z *= scalar; return *this; }

    Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
    Vector3& operator*=(const Vector3& other) { x *= other.x, y *= other.y, z *= other.z; return *this; }

    Vector3 operator*(const Quaternion& other) const; 

    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other) const {
        return x != other.x || y != other.y || z != other.z;
    }

};
