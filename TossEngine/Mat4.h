#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class TOSSENGINE_API Mat4 {
public:
    glm::mat4 value;

    Mat4() : value(1.0f) {}
    Mat4(float diagonal) : value(diagonal) {}
    Mat4(const glm::mat4& m) : value(m) {}

    operator glm::mat4() const { return value; }

    Mat4 operator*(const Mat4& other) const {
        return Mat4(value * other.value);
    }

    Mat4& operator*=(const Mat4& other) {
        value *= other.value;
        return *this;
    }

    static Mat4 Translate(const Vector3& translation) {
        return Mat4(glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(translation)));
    }

    static Mat4 Scale(const Vector3& scale) {
        return Mat4(glm::scale(glm::mat4(1.0f), static_cast<glm::vec3>(scale)));
    }

    static Mat4 Rotate(float angleRadians, const Vector3& axis) {
        return Mat4(glm::rotate(glm::mat4(1.0f), angleRadians, static_cast<glm::vec3>(axis)));
    }
};
