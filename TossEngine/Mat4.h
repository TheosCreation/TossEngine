#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class Vector3;
class Mat3;

class TOSSENGINE_API Mat4 {
public:
    glm::mat4 value;

    Mat4() : value(1.0f) {}
    Mat4(float diagonal) : value(diagonal) {}
    Mat4(const glm::mat4& m) : value(m) {}
    Mat4(const Mat3& mat);

    Mat4 Inverse();

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

    operator glm::mat4() const { return value; }

    Mat4 operator*(const Mat4& other) const {
        return Mat4(value * other.value);
    }

    Mat4& operator*=(const Mat4& other) {
        value *= other.value;
        return *this;
    }
    Vector3 operator*(const Vector3& vector) const;


    static Mat4 Translate(const Vector3& translation);

    static Mat4 Scale(const Vector3& scale);

    static Mat4 Rotate(float angleRadians, const Vector3& axis);
};
