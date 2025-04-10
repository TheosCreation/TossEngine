#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class Vector3;
class Mat4;

class TOSSENGINE_API Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
    Quaternion(const glm::quat& q) : w(q.w), x(q.x), y(q.y), z(q.z) {}
    Quaternion(const Vector3& eulerAngles);


    static Quaternion ExtractRotation(const Mat4& mat);

    // Convert to Mat4
    Mat4 ToMat4() const;
    Vector3 ToEulerAngles() const;

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
};