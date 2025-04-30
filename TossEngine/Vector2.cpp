#include "Vector2.h"
#include "Vector3.h"

Vector2 Vector2::operator+(const Vector3& other) const
{
    return Vector2(x + other.x, y + other.y);
}
