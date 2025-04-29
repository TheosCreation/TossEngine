/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Vector3.h
Description : 3D float vector class used for graphics, physics, and engine math operations.
              Wraps glm::vec3 and supports conversion to ReactPhysics3D's vector type.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"
#include "reactphysics3d/mathematics/Vector3.h"

// Forward declarations
class Mat4;
class Quaternion;
class Vector2;
class Vector4;

/**
 * @class Vector3
 * @brief A 3D vector class supporting arithmetic, normalization, conversion, and serialization.
 */
class TOSSENGINE_API Vector3 {
public:
    float x, y, z; //!< Components of the vector

    // --- Constructors ---

    constexpr Vector3() : x(0), y(0), z(0) {}
    constexpr Vector3(float val) : x(val), y(val), z(val) {}
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3(const glm::vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}
    Vector3(const reactphysics3d::Vector3& vec) : x(vec.x), y(vec.y), z(vec.z) {}
    Vector3(const Vector2& vec); // Presumably z = 0

    // --- Accessors ---

    float* Data() { return &x; }
    const float* Data() const { return &x; }

    // --- Math Operations ---

    float Length() const;
    Vector3 Normalized() const;
    void Normalize();

    static Vector3 Cross(const Vector3& a, const Vector3& b);
    static float Distance(const Vector3& a, const Vector3& b);

    static Vector3 ExtractTranslation(const Mat4& m);
    static Vector3 ExtractScale(const Mat4& m);

    static Vector3 Lerp(Vector3 start, Vector3 end, float t);

    Mat4 ToTranslation() const;
    Mat4 ToScale() const;

    bool Equals(const Vector3& other, float epsilon) const;

    Vector3 ToDegrees() const;
    Vector3 ToRadians() const;

    std::string ToString() const;

    // --- Type Conversion ---

    operator glm::vec3() const { return glm::vec3(x, y, z); }
    operator reactphysics3d::Vector3() const { return reactphysics3d::Vector3(x, y, z); }
    operator std::string() const { return ToString(); }

    // --- Operators ---

    Vector3 operator-() const;

    Vector3& operator-=(const Vector3& other);
    Vector3 operator-(const Vector3& other) const;

    Vector3 operator+(const Vector3& other) const;
    Vector3& operator+=(const Vector3& other);

    Vector3 operator*(float scalar) const;
    Vector3& operator*=(float scalar);

    Vector3 operator*(const Vector3& other) const;
    Vector3& operator*=(const Vector3& other);

    Vector3 operator/(const Vector3& other) const;
    Vector3& operator/=(const Vector3& other);

    Vector3 operator*(const Quaternion& other) const;

    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;

    // --- Constants ---
    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 Forward;
    static const Vector3 Back;
    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Right;
    static const Vector3 Left;
};

// --- JSON Serialization ---

inline void to_json(json& j, const Vector3& v) {
    j = json{ { "x", v.x }, { "y", v.y }, { "z", v.z } };
}

inline void from_json(const json& j, Vector3& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
    if (j.contains("z") && !j["z"].is_null()) j.at("z").get_to(v.z);
}