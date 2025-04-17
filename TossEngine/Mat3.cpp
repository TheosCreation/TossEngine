#include "Mat3.h"
#include "Mat4.h"
#include "Vector3.h"

Mat3::Mat3(const Mat4& mat)
{
    value = glm::mat3(mat.value);
}

Mat3::Mat3(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
    value = glm::mat3(v1, v2, v3);
}
