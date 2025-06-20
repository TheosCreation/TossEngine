#include "Quaternion.h"
#include "Mat4.h"
#include "Mat3.h"
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

Quaternion::Quaternion(const Mat3& mat)
{
}

Quaternion Quaternion::ExtractRotation(const Mat4& mat)
{
    glm::mat m = mat.value;
    Vector3 scale = Vector3::ExtractScale(m);
    float r00 = m[0][0] / scale.x;
    float r01 = m[0][1] / scale.y;
    float r02 = m[0][2] / scale.z;

    float r10 = m[1][0] / scale.x;
    float r11 = m[1][1] / scale.y;
    float r12 = m[1][2] / scale.z;

    float r20 = m[2][0] / scale.x;
    float r21 = m[2][1] / scale.y;
    float r22 = m[2][2] / scale.z;

    float trace = r00 + r11 + r22;
    float qw, qx, qy, qz;

    if (trace > 0.0f)
    {
        float s = sqrt(trace + 1.0f) * 2.0f; // s=4*qw
        qw = 0.25f * s;
        qx = (r21 - r12) / s;
        qy = (r02 - r20) / s;
        qz = (r10 - r01) / s;
    }
    else if ((r00 > r11) && (r00 > r22))
    {
        float s = sqrt(1.0f + r00 - r11 - r22) * 2.0f; // s=4*qx
        qw = (r21 - r12) / s;
        qx = 0.25f * s;
        qy = (r01 + r10) / s;
        qz = (r02 + r20) / s;
    }
    else if (r11 > r22)
    {
        float s = sqrt(1.0f + r11 - r00 - r22) * 2.0f; // s=4*qy
        qw = (r02 - r20) / s;
        qx = (r01 + r10) / s;
        qy = 0.25f * s;
        qz = (r12 + r21) / s;
    }
    else
    {
        float s = sqrt(1.0f + r22 - r00 - r11) * 2.0f; // s=4*qz
        qw = (r10 - r01) / s;
        qx = (r02 + r20) / s;
        qy = (r12 + r21) / s;
        qz = 0.25f * s;
    }

    return Quaternion(qw, qx, qy, qz);
}

Quaternion Quaternion::Identity()
{
    return Quaternion();
}

Mat4 Quaternion::ToMat4() const
{
    return Mat4(glm::toMat4(static_cast<glm::quat>(*this)));
}

Vector3 Quaternion::ToEulerAngles() const {
    // Returns Euler angles (in radians) from this quaternion
    return Vector3(glm::eulerAngles(static_cast<glm::quat>(*this)));
}
float Quaternion::Magnitude() const
{
    return glm::length(static_cast<glm::quat>(*this));
}


Vector3 Quaternion::operator*(const Vector3& v) const
{
    glm::vec3 rotated = static_cast<glm::quat>(*this) * static_cast<glm::vec3>(v);
    return Vector3(rotated);
}

Quaternion Quaternion::FromEuler(Vector3 eulerAngles)
{
    return Quaternion(eulerAngles);
}

Quaternion Quaternion::FromLookDirection(const Vector3& forward, const Vector3& up)
{
    Vector3 f = forward.Normalized();
    Vector3 r = Vector3::Cross(up, f).Normalized(); // right
    Vector3 u = Vector3::Cross(f, r);                // real up

    Mat3 rotMatrix;
    rotMatrix.value[0] = r;
    rotMatrix.value[1] = u;
    rotMatrix.value[2] = f;

    return Quaternion(rotMatrix);
}

Quaternion Quaternion::LookAt(const Vector3& direction, const Vector3& up)
{
    // Ensure the direction is normalized
    Vector3 forward = direction.Normalized();

    Vector3 right = Vector3::Cross(up, forward).Normalized();

    Vector3 recalculatedUp = Vector3::Cross(forward, right).Normalized();

    // Create a 3x3 rotation matrix
    Mat3 rotationMatrix(right, recalculatedUp, forward);

    // Convert the matrix to a quaternion
    return Quaternion(rotationMatrix);
}

Quaternion Quaternion::Slerp(const Quaternion& quat1, const Quaternion& quat2, float t)
{
    return glm::slerp(static_cast<glm::quat>(quat1), static_cast<glm::quat>(quat2), t);
}
