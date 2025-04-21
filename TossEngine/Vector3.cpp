#include "Vector3.h"
#include "Vector2.h"
#include "Mat4.h"
#include "Quaternion.h"

const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Back = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);

Vector3::Vector3(const Vector2& vec) : x(vec.x), y(vec.y), z(0.0f)
{
}

Vector3 Vector3::ExtractTranslation(const Mat4& m)
{
    return Vector3(m.value[3][0], m.value[3][1], m.value[3][2]);
}

Vector3 Vector3::ExtractScale(const Mat4& m)
{
    glm::mat4 glmMat = m.value;
    Vector3 scale;
    scale.x = glmMat[0][0] * glmMat[0][0] + glmMat[1][0] * glmMat[1][0] + glmMat[2][0] * glmMat[2][0];
    scale.x = sqrt(scale.x);

    scale.y = glmMat[0][1] * glmMat[0][1] + glmMat[1][1] * glmMat[1][1] + glmMat[2][1] * glmMat[2][1];
    scale.y = sqrt(scale.y);

    scale.z = glmMat[0][2] * glmMat[0][2] + glmMat[1][2] * glmMat[1][2] + glmMat[2][2] * glmMat[2][2];
    scale.z = sqrt(scale.z);

    return scale;
}

Vector3 Vector3::Lerp(Vector3 startVector, Vector3 endVector, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);

    return startVector + (endVector - startVector) * t;
}

Mat4 Vector3::ToTranslation() const
{
    return Mat4(glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(*this)));
}

Mat4 Vector3::ToScale() const
{
    return Mat4(glm::scale(glm::mat4(1.0f), static_cast<glm::vec3>(*this)));
}

Vector3 Vector3::operator*(const Quaternion& other) const
{
    return Vector3(x * other.x, y * other.y, z * other.z);
}