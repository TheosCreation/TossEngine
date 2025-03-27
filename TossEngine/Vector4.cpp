#include "Vector4.h"
#include "Vector3.h"

constexpr Vector4::Vector4(const Vector3& v, float w)

    : x(v.x), y(v.y), z(v.z), w(w) {
}