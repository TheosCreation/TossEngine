/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Rect.h
Description : A rectangle container class that stores position and size using integer coordinates.
              Useful for scissor rects, viewport regions, or UI element bounds.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Math.h"

// Forward declaration
class Vector2;

/**
 * @class Rect
 * @brief A simple rectangle class representing position (left, top) and size (width, height).
 */
class Rect {
public:
    // --- Fields ---

    int width = 0; //!< The width of the rectangle
    int height = 0; //!< The height of the rectangle
    int left = 0; //!< The x-coordinate of the left side
    int top = 0; //!< The y-coordinate of the top side

    // --- Constructors ---

    /**
     * @brief Default constructor. Initializes to zero.
     */
    Rect() = default;

    /**
     * @brief Initializes the rectangle with width and height at origin (0, 0).
     */
    Rect(int width, int height) : width(width), height(height) {}

    /**
     * @brief Initializes the rectangle with position and size.
     */
    Rect(int left, int top, int width, int height)
        : width(width), height(height), left(left), top(top) {
    }

    /**
     * @brief Copy constructor.
     */
    Rect(const Rect& rect)
        : width(rect.width), height(rect.height), left(rect.left), top(rect.top) {
    }

    /**
     * @brief Initializes the rectangle from position and size as Vector2.
     */
    Rect(const Vector2& position, const Vector2& size);

    // --- Methods ---

    /**
     * @brief Checks whether a point lies inside the rectangle.
     * @param vector The point to test.
     * @return True if the point is inside; otherwise false.
     */
    bool Contains(Vector2 vector) const;

    /**
     * @brief Converts the rectangle to a human-readable string.
     * @return A formatted string in (left, top, width, height) form.
     */
    std::string ToString() const {
        std::ostringstream oss;
        oss << "(" << left << ", " << top << ", " << width << ", " << height << ")";
        return oss.str();
    }
};

// --- JSON Serialization ---

/**
 * @brief Serializes the Rect object to a JSON object.
 */
inline void to_json(json& j, Rect const& r) {
    j = json{
        { "left",   r.left   },
        { "top",    r.top    },
        { "width",  r.width  },
        { "height", r.height }
    };
}

/**
 * @brief Deserializes a JSON object to a Rect.
 */
inline void from_json(json const& j, Rect& r) {
    j.at("left").get_to(r.left);
    j.at("top").get_to(r.top);
    j.at("width").get_to(r.width);
    j.at("height").get_to(r.height);
}