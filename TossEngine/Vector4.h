/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Vector4.h
Description : A 4D float vector class supporting common math operations and JSON serialization.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class Vector3;

/**
 * @class Vector4
 * @brief A 4-component floating-point vector for graphics and math operations.
 */
class TOSSENGINE_API Vector4 {
public:
    float x, y, z, w;

    // --- Constructors ---
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float val) : x(val), y(val), z(val), w(val) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    constexpr Vector4(const Vector3& v, float w);

    // --- Accessors ---
    float* Data() { return &x; }
    const float* Data() const { return &x; }

    // --- Math Helpers ---
    float Length() const;
    Vector4 Normalized() const;
    void Normalize();

    // --- Conversion ---
    operator glm::vec4() const { return glm::vec4(x, y, z, w); }

    // --- String Conversion ---
    std::string ToString() const;

    // --- Operators ---
    Vector4 operator-() const;
    Vector4& operator-=(const Vector4& other);

    Vector4 operator+(const Vector4& other) const;
    Vector4& operator+=(const Vector4& other);

    Vector4 operator-(const Vector4& other) const;
    Vector4 operator*(float scalar) const;
    Vector4& operator*=(float scalar);

    Vector4 operator/(float scalar) const;
    Vector4& operator/=(float scalar);

    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;
};

// --- JSON Serialization ---

inline void to_json(json& j, const Vector4& v) {
    j = json{ { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
}

inline void from_json(const json& j, Vector4& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
    if (j.contains("z") && !j["z"].is_null()) j.at("z").get_to(v.z);
    if (j.contains("w") && !j["w"].is_null()) j.at("w").get_to(v.w);
}
