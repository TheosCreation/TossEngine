#include "Mat4.h"
#include "Vector3.h"
#include "Mat3.h"
#include "Vector4.h"
#include "Quaternion.h"

Mat4::Mat4(const Mat3& mat)
{
    value = glm::mat4(mat.value);
}

Mat4::Mat4(const Quaternion& quat)
{
    // Convert your custom Quaternion to glm::quat
    glm::quat glmQuat(quat.w, quat.x, quat.y, quat.z);

    // Use glm::mat4_cast to convert the quaternion to a 4x4 rotation matrix
    value = glm::mat4_cast(glmQuat);
}

Mat4 Mat4::Inverse()
{
    return glm::inverse(value);
}

Vector3 Mat4::operator*(const Vector3& vector) const
{
    // Convert the Vector3 to a homogeneous 4D vector (w = 1)
    Vector4 vec4(vector.x, vector.y, vector.z, 1.0f);

    // Perform matrix-vector multiplication
    float x = value[0][0] * vec4.x + value[0][1] * vec4.y + value[0][2] * vec4.z + value[0][3] * vec4.w;
    float y = value[1][0] * vec4.x + value[1][1] * vec4.y + value[1][2] * vec4.z + value[1][3] * vec4.w;
    float z = value[2][0] * vec4.x + value[2][1] * vec4.y + value[2][2] * vec4.z + value[2][3] * vec4.w;
    float w = value[3][0] * vec4.x + value[3][1] * vec4.y + value[3][2] * vec4.z + value[3][3] * vec4.w;

    return Vector3(x, y, z);
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

Vector3 Mat4::TransformPoint(const Vector3& point) const
{
    glm::vec4 temp = value * glm::vec4(point.x, point.y, point.z, 1.0f);
    return Vector3(temp.x, temp.y, temp.z);
}