#include "Rect.h"
#include "Vector2.h"

Rect::Rect(const Vector2& position, const Vector2& size)
    : left(static_cast<int>(position.x)),
    top(static_cast<int>(position.y)),
    width(static_cast<int>(size.x)),
    height(static_cast<int>(size.y)) {
}

bool Rect::Contains(Vector2 vector) const
{
    return (vector.x >= left) && (vector.x <= left + width) &&
        (vector.y >= top) && (vector.y <= top + height);
}
