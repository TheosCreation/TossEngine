/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Mat4.h
Description : Represents a 4x4 matrix used for transformation in 3D space.
              Wraps glm::mat4 and exposes core mathematical operations for translation, rotation, and scaling.
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"

// Forward declarations to avoid circular dependency
class Vector3;
class Mat3;
class Quaternion;

/**
 * @class Mat4
 * @brief Wrapper around glm::mat4 to represent a 4x4 transformation matrix.
 *        Used for 3D object transformations and camera calculations.
 */
class TOSSENGINE_API Mat4 {
public:
    glm::mat4 value; //!< Underlying GLM matrix value.

    /**
     * @brief Constructs an identity matrix.
     */
    Mat4() : value(1.0f) {}

    /**
     * @brief Constructs a matrix with a diagonal value.
     * @param diagonal Value to fill along the main diagonal.
     */
    Mat4(float diagonal) : value(diagonal) {}

    /**
     * @brief Constructs a Mat4 from an existing glm::mat4.
     * @param m Source glm matrix.
     */
    Mat4(const glm::mat4& m) : value(m) {}

    /**
     * @brief Constructs a 4x4 matrix from a 3x3 matrix, embedding into the upper-left.
     * @param mat The 3x3 matrix to embed.
     */
    Mat4(const Mat3& mat);

    /**
     * @brief Constructs a rotation matrix from a quaternion.
     * @param quat The quaternion to convert.
     */
    Mat4(const Quaternion& quat);

    /**
     * @brief Returns the inverse of this matrix.
     * @return A new Mat4 that is the inverse of this matrix.
     */
    Mat4 Inverse();

    /**
     * @brief Transforms a 3D point using this matrix (assumes w = 1).
     * @param point The point to transform.
     * @return Transformed point in 3D space.
     */
    Vector3 TransformPoint(const Vector3& point) const;

    /**
     * @brief Converts this matrix into a string representation for debugging.
     * @return A string displaying the 4x4 matrix layout.
     */
    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "[\n";
        for (int i = 0; i < 4; ++i) {
            oss << "  [ ";
            for (int j = 0; j < 4; ++j) {
                oss << value[i][j] << " ";
            }
            oss << "]";
            if (i != 3)
                oss << "\n";
        }
        oss << "\n]";
        return oss.str();
    }
    
    /**
         * @brief Returns a mutable pointer to the underlying matrix data.
         * @return Pointer to 16 contiguous floats.
         */
    float* data()
    {
        return glm::value_ptr(value);
    }

    /**
     * @brief Returns a read-only pointer to the underlying matrix data.
     * @return Pointer to 16 contiguous floats.
     */
    const float* data() const
    {
        return glm::value_ptr(value);
    }
    
    /**
     * @brief Implicit conversion to glm::mat4 for interoperability.
     */
    operator glm::mat4() const { return value; }

    /**
     * @brief Matrix multiplication with another Mat4.
     * @param other The right-hand side matrix.
     * @return A new Mat4 representing the product.
     */
    Mat4 operator*(const Mat4& other) const {
        return Mat4(value * other.value);
    }

    /**
     * @brief In-place matrix multiplication.
     * @param other The matrix to multiply by.
     * @return Reference to the updated matrix.
     */
    Mat4& operator*=(const Mat4& other) {
        value *= other.value;
        return *this;
    }

    /**
     * @brief Multiplies this matrix with a Vector3 (assumes w = 1).
     * @param vector The vector to transform.
     * @return Transformed vector.
     */
    Vector3 operator*(const Vector3& vector) const;

    /**
     * @brief Creates a translation matrix.
     * @param translation The amount to translate.
     * @return A Mat4 representing the translation.
     */
    static Mat4 Translate(const Vector3& translation);

    /**
     * @brief Creates a scaling matrix.
     * @param scale The scale factors along x, y, z.
     * @return A Mat4 representing the scaling.
     */
    static Mat4 Scale(const Vector3& scale);

    /**
     * @brief Creates a rotation matrix around an axis.
     * @param angleRadians Angle in radians.
     * @param axis The axis to rotate around.
     * @return A Mat4 representing the rotation.
     */
    static Mat4 Rotate(float angleRadians, const Vector3& axis);
};



inline void to_json(json& j, const Mat4& m)
{
    j = json::array();
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            j.push_back(m.value[r][c]);
        }
    }
}

inline void from_json(const json& j, Mat4& m)
{
    int idx = 0;
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            m.value[r][c] = j.at(idx).get<float>();
            idx += 1;
        }
    }
}