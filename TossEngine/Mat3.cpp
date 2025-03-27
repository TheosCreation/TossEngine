#include "Mat3.h"
#include "Mat4.h"

Mat3::Mat3(const Mat4& mat)
{
    value = glm::mat3(mat.value);
}
