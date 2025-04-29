/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Math.h
Description : Central math utility header that exposes GLM and provides TossEngine-specific math tools,
              random generation, clamping, and JSON aliasing.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

// Core GLM includes
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/component_wise.hpp>
#include <gtx/quaternion.hpp>

// Standard includes
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <string>

// JSON (external)
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Mathematical constants
#ifndef PI
#define PI 3.14159265358979323846
#endif

/**
 * @brief Initializes the random seed using the current time.
 *        Call once during application startup to ensure variability.
 */
inline void initRandomSeed()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}

/**
 * @brief Generates a random float between 0 and max.
 * @param max The upper bound of the range.
 * @return Random float between [0, max].
 */
inline float randomNumber(float max)
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * max;
}

/**
 * @brief Generates a random float between min and max.
 * @param min The lower bound of the range.
 * @param max The upper bound of the range.
 * @return Random float between [min, max].
 */
inline float randomRange(float min, float max)
{
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

/**
 * @brief Clamps a value between a given min and max using std::clamp.
 * @tparam T The type being clamped (must support comparison).
 * @param value The value to clamp.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return The clamped value.
 */
template<typename T>
constexpr const T& Clamp(const T& value, const T& min, const T& max) {
    return std::clamp(value, min, max);
}
