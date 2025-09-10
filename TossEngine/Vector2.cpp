#include "Vector2.h"
#include "Vector3.h"

float Vector2::Length() const
{
#if defined(_WIN32)
    return glm::length(static_cast<glm::vec2>(*this));
#elif defined(__PROSPERO__)
    return 0.0f; //TODO: Implement
#endif
    return 0.0f;
}

Vector2 Vector2::Normalized() const
{
#if defined(_WIN32)
    glm::vec2 v = glm::normalize(static_cast<glm::vec2>(*this));
    return Vector2(v);
#elif defined(__PROSPERO__)
    return *this; //TODO: Implement
#endif

    return *this;
}

Vector2 Vector2::operator+(const Vector3& other) const
{
    return Vector2(x + other.x, y + other.y);
}
