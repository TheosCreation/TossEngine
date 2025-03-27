#include "Mat4.h"
#include "Vector3.h"
#include "Mat3.h"

Mat4::Mat4(const Mat3& mat)
{
    value = glm::mat4(mat.value);
}

Mat4 Mat4::Translate(const Vector3& translation)
{
    return Mat4(glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(translation)));
}

Mat4 Mat4::Scale(const Vector3& scale)
{
    return Mat4(glm::scale(glm::mat4(1.0f), static_cast<glm::vec3>(scale)));
}

Mat4 Mat4::Rotate(float angleRadians, const Vector3& axis)
{
    return Mat4(glm::rotate(glm::mat4(1.0f), angleRadians, static_cast<glm::vec3>(axis)));
}