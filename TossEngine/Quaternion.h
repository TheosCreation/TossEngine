#pragma once
#include "TossEngineAPI.h"
#include "Math.h"
#include "Vector3.h"

class Vector3;
class Mat4;
class Mat3;

class TOSSENGINE_API Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
    Quaternion(const glm::quat& q) : w(q.w), x(q.x), y(q.y), z(q.z) {}
    Quaternion(const Vector3& eulerAngles);
    Quaternion(const Mat3& mat);


    static Quaternion ExtractRotation(const Mat4& mat);
    static Quaternion Identity();

    // Convert to Mat4
    Mat4 ToMat4() const;
    Vector3 ToEulerAngles() const;
    float Magnitude() const;

    // Normalize this quaternion
    void Normalize() {
        glm::quat q = glm::normalize(static_cast<glm::quat>(*this));
        w = q.w; x = q.x; y = q.y; z = q.z;
    }

    // Return a normalized copy
    Quaternion Normalized() const {
        glm::quat q = glm::normalize(static_cast<glm::quat>(*this));
        return Quaternion(q);
    }
    
    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "(" << w << ", " << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    // Cast to glm::quat
    operator glm::quat() const { return glm::quat(w, x, y, z); }

    // Multiply (concatenate) quaternions
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(static_cast<glm::quat>(*this) * static_cast<glm::quat>(other));
    }

    // Rotate a vector
    Vector3 operator*(const Vector3& v) const;

    static Quaternion FromEuler(Vector3 eulerAngles);
    static Quaternion FromLookDirection(const Vector3& forward, const Vector3& up);
    static Quaternion LookAt(const Vector3& direction, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));
    static Quaternion Slerp(const Quaternion& quat1, const Quaternion& quat2, float t);
};