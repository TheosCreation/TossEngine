#pragma once
#include "TossEngineAPI.h"
#include "Math.h"

class TOSSENGINE_API Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
    Quaternion(const glm::quat& q) : w(q.w), x(q.x), y(q.y), z(q.z) {}

    operator glm::quat() const { return glm::quat(w, x, y, z); }
};
