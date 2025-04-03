#include "Vector3.h"
#include "Vector2.h"
#include "Mat4.h"
#include "Quaternion.h"

Vector3::Vector3(const Vector2& vec) : x(vec.x), y(vec.y), z(0.0f)
{
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