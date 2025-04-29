/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Mat3.h
Description : Lightweight wrapper for 3x3 matrices using GLM.
              Supports DLL export and TossEngine internal math compatibility.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"

// Forward declarations
class Mat4;
class Vector3;

/**
 * @class Mat3
 * @brief Represents a 3x3 matrix used for transformations, normal calculations, and more.
 *        Wraps a glm::mat3 and adds support for engine-level integration and operator overloading.
 */
class TOSSENGINE_API Mat3 {
public:
    glm::mat3 value; //!< Internal matrix data.

    /**
     * @brief Default constructor (identity matrix).
     */
    Mat3() : value(1.0f) {}

    /**
     * @brief Constructs a matrix with a diagonal value.
     * @param diagonal Value to set on the diagonal.
     */
    explicit Mat3(float diagonal) : value(diagonal) {}

    /**
     * @brief Constructs from a glm::mat3.
     * @param m Input matrix.
     */
    explicit Mat3(const glm::mat3& m) : value(m) {}

    /**
     * @brief Constructs from a Mat4 (extracting top-left 3x3).
     * @param mat The 4x4 matrix to convert from.
     */
    Mat3(const Mat4& mat);

    /**
     * @brief Constructs a matrix from three Vector3 column vectors.
     * @param v1 First column.
     * @param v2 Second column.
     * @param v3 Third column.
     */
    Mat3(const Vector3& v1, const Vector3& v2, const Vector3& v3);

    /**
     * @brief Converts this matrix to a glm::mat3.
     */
    operator glm::mat3() const { return value; }

    /**
     * @brief Matrix multiplication.
     * @param other The right-hand matrix.
     * @return Resulting Mat3.
     */
    Mat3 operator*(const Mat3& other) const {
        return Mat3(value * other.value);
    }

    /**
     * @brief Matrix multiplication and assignment.
     * @param other The right-hand matrix.
     * @return Reference to this Mat3.
     */
    Mat3& operator*=(const Mat3& other) {
        value *= other.value;
        return *this;
    }
};