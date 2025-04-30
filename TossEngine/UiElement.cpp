#include "UiElement.h"

Vector2 UiElement::GetAnchorOffset(const Vector2& screenSize, const Vector2& size, AnchorPoint anchor)
{
    Vector2 offset;

    switch (anchor)
    {
    case AnchorPoint::TopLeft:
        offset = Vector2(0, 0);
        break;
    case AnchorPoint::TopCenter:
        offset = Vector2((screenSize.x - size.x) / 2.0f, 0);
        break;
    case AnchorPoint::TopRight:
        offset = Vector2(screenSize.x - size.x, 0);
        break;
    case AnchorPoint::MiddleLeft:
    case AnchorPoint::Center:
        offset = Vector2((screenSize.x - size.x) / 2.0f, (screenSize.y - size.y) / 2.0f);
        break;
    case AnchorPoint::MiddleRight:
        offset = Vector2(screenSize.x - size.x, (screenSize.y - size.y) / 2.0f);
        break;
    case AnchorPoint::BottomLeft:
        offset = Vector2(0, screenSize.y - size.y);
        break;
    case AnchorPoint::BottomCenter:
        offset = Vector2((screenSize.x - size.x) / 2.0f, screenSize.y - size.y);
        break;
    case AnchorPoint::BottomRight:
        offset = Vector2(screenSize.x - size.x, screenSize.y - size.y);
        break;
    }

    return offset;
}