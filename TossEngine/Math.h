/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Math.h
Description : allow other classes to include the glm library and this header adds more specific required functionality
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/component_wise.hpp>
#include <gtx/quaternion.hpp>

#define PI 3.14159265358979323846

inline void initRandomSeed()
{
    srand(static_cast<unsigned int>(time(0)));
}


/**
 * @brief Generates a random number between 0 and max.
 * @param max The largest number that can be generated.
 */
inline float randomNumber(float max)
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * max;
}

/**
 * @brief Generates a random number between min and max.
 * @param min The least number that can be generated.
 * @param max The largest number that can be generated.
 */
inline float randomRange(float min, float max)
{
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

template<typename T>
constexpr const T& Clamp(const T& value, const T& min, const T& max) {
    return std::clamp(value, min, max);
}