/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Vector2.h
Description : Represents a 2D float-based vector with basic arithmetic and serialization support.
              Wraps glm::vec2 for math operations and engine compatibility.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"

class Vector3;

/**
 * @class Vector2
 * @brief A 2D vector structure for use in math, UI, and graphics operations.
 */
class TOSSENGINE_API Vector2 {
public:
    float x, y; //!< X and Y components of the vector

    // --- Constructors ---

    /**
     * @brief Default constructor (zero vector).
     */
    Vector2() : x(0), y(0) {}

    /**
     * @brief Constructs both components with a single float value.
     */
    Vector2(float val) : x(val), y(val) {}

    /**
     * @brief Constructs with individual X and Y values.
     */
    Vector2(float x, float y) : x(x), y(y) {}

    /**
     * @brief Constructs from a glm::vec2.
     */
    Vector2(const glm::vec2& v) : x(v.x), y(v.y) {}

    // --- Utility Functions ---

    /**
     * @brief Returns a raw pointer to the data (x, y).
     */
    float* Data() { return &x; }

    /**
     * @brief Returns a const pointer to the data (x, y).
     */
    const float* Data() const { return &x; }

    /**
     * @brief Computes the length (magnitude) of the vector.
     */
    float Length() const { return glm::length(static_cast<glm::vec2>(*this)); }

    /**
     * @brief Returns a normalized (unit length) copy of this vector.
     */
    Vector2 Normalized() const {
        glm::vec2 v = glm::normalize(static_cast<glm::vec2>(*this));
        return Vector2(v);
    }

    /**
     * @brief Returns a string representation of the vector.
     */
    std::string ToString() const {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ")";
        return oss.str();
    }

    // --- Type Conversions ---

    /**
     * @brief Implicit conversion to glm::vec2.
     */
    operator glm::vec2() const { return glm::vec2(x, y); }

    /**
     * @brief Implicit conversion to std::string for debugging.
     */
    operator std::string() const { return ToString(); }

    // --- Operators ---

    /**
     * @brief Unary negation operator.
     */
    Vector2 operator-() const { return Vector2(-x, -y); }

    /**
     * @brief Component-wise multiplication.
     */
    Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }

    /**
     * @brief Component-wise addition.
     */
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }

    /**
     * @brief Component-wise addition with Vector3 conversion.
     */
    Vector2 operator+(const Vector3& other) const;

    /**
     * @brief Component-wise subtraction assignment.
     */
    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /**
     * @brief Equality comparison.
     */
    bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }

    /**
     * @brief Inequality comparison.
     */
    bool operator!=(const Vector2& other) const { return !(*this == other); }
};

// --- JSON Serialization ---

/**
 * @brief Serializes a Vector2 to JSON.
 */
inline void to_json(json& j, const Vector2& v) {
    j = json{ { "x", v.x }, { "y", v.y } };
}

/**
 * @brief Deserializes a Vector2 from JSON.
 */
inline void from_json(const json& j, Vector2& v) {
    if (j.contains("x") && !j["x"].is_null()) j.at("x").get_to(v.x);
    if (j.contains("y") && !j["y"].is_null()) j.at("y").get_to(v.y);
}