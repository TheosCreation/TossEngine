#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class Mat4;
class Vector3;

class TOSSENGINE_API Mat3 {
public:
    glm::mat3 value;

    Mat3() : value(1.0f) {}
    Mat3(float diagonal) : value(diagonal) {}
    Mat3(const glm::mat3& m) : value(m) {}
    Mat3(const Mat4& mat);
    Mat3(const Vector3& v1,const Vector3& v2, const Vector3& v3);

    operator glm::mat3() const { return value; }

    Mat3 operator*(const Mat3& other) const {
        return Mat3(value * other.value);
    }

    Mat3& operator*=(const Mat3& other) {
        value *= other.value;
        return *this;
    }
};
