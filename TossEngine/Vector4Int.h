/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Vector4.h
Description : A 4D float vector class supporting common math operations and JSON serialization.
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

/**
 * @class Vector4Int
 * @brief A 4-component int vector for graphics and math operations.
 */
class TOSSENGINE_API Vector4Int {
public:
    int x, y, z, w;

    // --- Constructors ---
    Vector4Int() : x(0), y(0), z(0), w(0) {}
    Vector4Int(int val) : x(val), y(val), z(val), w(val) {}
    Vector4Int(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
    
    // --- String Conversion ---
    std::string ToString() const;

    // --- Operators ---
    //Vector4Int operator-() const;
    //Vector4Int& operator-=(const Vector4Int& other);
//
    //Vector4Int operator+(const Vector4Int& other) const;
    //Vector4Int& operator+=(const Vector4Int& other);
//
    //Vector4Int operator-(const Vector4Int& other) const;
    //Vector4Int operator*(float scalar) const;
    //Vector4Int& operator*=(float scalar);
//
    //Vector4Int operator/(float scalar) const;
    //Vector4Int& operator/=(float scalar);
//
    //bool operator==(const Vector4Int& other) const;
    //bool operator!=(const Vector4Int& other) const;
};

// --- JSON Serialization ---

inline void to_json(json& j, const Vector4Int& v) {
    j = json{ { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
}

inline void from_json(const json& j, Vector4Int& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
    if (j.contains("z") && !j["z"].is_null()) j.at("z").get_to(v.z);
    if (j.contains("w") && !j["w"].is_null()) j.at("w").get_to(v.w);
}