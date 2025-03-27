#include "Quaternion.h"
#include "Mat4.h"
#include "Vector3.h"

Quaternion::Quaternion(const Vector3& eulerAngles)
{
    glm::vec3 radians = static_cast<glm::vec3>(eulerAngles);
    glm::quat q = glm::quat(radians);
    w = q.w;
    x = q.x;
    y = q.y;
    z = q.z;
}

Mat4 Quaternion::ToMat4() const
{
    return Mat4(glm::toMat4(static_cast<glm::quat>(*this)));
}

Vector3 Quaternion::ToEulerAngles() const {
    // Returns Euler angles (in radians) from this quaternion
    return Vector3(glm::eulerAngles(static_cast<glm::quat>(*this)));
} 

Vector3 Quaternion::operator*(const Vector3& v) const
{
    glm::vec3 rotated = static_cast<glm::quat>(*this) * static_cast<glm::vec3>(v);
    return Vector3(rotated);
}