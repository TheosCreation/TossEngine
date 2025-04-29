/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Quaternion.h
Description : Represents a quaternion for 3D rotation. Wraps a float-based structure and provides
              conversion, normalization, and rotation utility functions.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"
#include "Math.h"
#include "Vector3.h"

// Forward declarations
class Vector3;
class Mat4;
class Mat3;

/**
 * @class Quaternion
 * @brief Represents a quaternion for 3D rotation. Supports conversions to/from matrices and Euler angles,
 *        normalization, interpolation, and rotation of vectors.
 */
class TOSSENGINE_API Quaternion {
public:
    float w, x, y, z; //!< Quaternion components: w (scalar), x/y/z (vector)

    // --- Constructors ---

    /**
     * @brief Default constructor (identity rotation).
     */
    Quaternion() : w(1), x(0), y(0), z(0) {}

    /**
     * @brief Constructs a quaternion from raw float components.
     */
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    /**
     * @brief Constructs a quaternion from a glm::quat.
     */
    Quaternion(const glm::quat& q) : w(q.w), x(q.x), y(q.y), z(q.z) {}

    /**
     * @brief Constructs a quaternion from Euler angles in radians.
     */
    Quaternion(const Vector3& eulerAngles);

    /**
     * @brief Constructs a quaternion from a 3x3 rotation matrix.
     */
    Quaternion(const Mat3& mat);

    // --- Static Factory Methods ---

    /**
     * @brief Returns a quaternion representing no rotation.
     */
    static Quaternion Identity();

    /**
     * @brief Extracts the rotation portion of a 4x4 transformation matrix.
     */
    static Quaternion ExtractRotation(const Mat4& mat);

    /**
     * @brief Creates a quaternion from Euler angles (XYZ order), in radians.
     */
    static Quaternion FromEuler(Vector3 eulerAngles);

    /**
     * @brief Creates a quaternion that rotates from world forward to given direction.
     */
    static Quaternion FromLookDirection(const Vector3& forward, const Vector3& up);

    /**
     * @brief Creates a quaternion that rotates the +Z axis to point in the direction specified.
     */
    static Quaternion LookAt(const Vector3& direction, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));

    /**
     * @brief Spherical linear interpolation between two quaternions.
     * @param quat1 First quaternion.
     * @param quat2 Second quaternion.
     * @param t Interpolation value [0, 1].
     * @return Interpolated quaternion.
     */
    static Quaternion Slerp(const Quaternion& quat1, const Quaternion& quat2, float t);

    // --- Conversion Methods ---

    /**
     * @brief Converts this quaternion to a 4x4 rotation matrix.
     */
    Mat4 ToMat4() const;

    /**
     * @brief Converts this quaternion to Euler angles (XYZ order, in radians).
     */
    Vector3 ToEulerAngles() const;

    /**
     * @brief Converts this quaternion to a readable string.
     */
    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "(" << w << ", " << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    // --- Normalization ---

    /**
     * @brief Returns the magnitude (length) of the quaternion.
     */
    float Magnitude() const;

    /**
     * @brief Normalizes this quaternion in place.
     */
    void Normalize() {
        glm::quat q = glm::normalize(static_cast<glm::quat>(*this));
        w = q.w; x = q.x; y = q.y; z = q.z;
    }

    /**
     * @brief Returns a normalized copy of this quaternion.
     */
    Quaternion Normalized() const {
        glm::quat q = glm::normalize(static_cast<glm::quat>(*this));
        return Quaternion(q);
    }

    // --- Operators ---

    /**
     * @brief Implicit conversion to glm::quat for GLM interoperability.
     */
    operator glm::quat() const { return glm::quat(w, x, y, z); }

    /**
     * @brief Concatenates this quaternion with another.
     *        Equivalent to applying the other rotation after this one.
     */
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(static_cast<glm::quat>(*this) * static_cast<glm::quat>(other));
    }

    /**
     * @brief Rotates a vector using this quaternion.
     */
    Vector3 operator*(const Vector3& v) const;
};