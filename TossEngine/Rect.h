/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Rect.h
Description : a rectangle container class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Math.h"

/**
 * @class Rect
 * @brief A rectangle container class.
 */
class Rect
{
public:
    /**
     * @brief Default constructor for the Rect class.
     */
    Rect() {}

    /**
     * @brief Constructor for the Rect class with width and height.
     * @param width The width of the rectangle.
     * @param height The height of the rectangle.
     */
    Rect(int width, int height) : width(width), height(height) {}

    /**
     * @brief Constructor for the Rect class with position and size.
     * @param left The left position of the rectangle.
     * @param top The top position of the rectangle.
     * @param width The width of the rectangle.
     * @param height The height of the rectangle.
     */
    Rect(int left, int top, int width, int height) : left(left), top(top), width(width), height(height) {}

    /**
     * @brief Copy constructor for the Rect class.
     * @param rect The rectangle to copy.
     */
    Rect(const Rect& rect) : left(rect.left), top(rect.top), width(rect.width), height(rect.height) {}

public:
    int width = 0;  // The width of the rectangle
    int height = 0; // The height of the rectangle
    int left = 0;   // The left position of the rectangle
    int top = 0;    // The top position of the rectangle
};

inline void to_json(json& j, Rect const& r) {
    j = json{
        { "left",   r.left   },
        { "top",    r.top    },
        { "width",  r.width  },
        { "height", r.height }
    };
}

inline void from_json(json const& j, Rect& r) {
    j.at("left").get_to(r.left);
    j.at("top").get_to(r.top);
    j.at("width").get_to(r.width);
    j.at("height").get_to(r.height);
}
