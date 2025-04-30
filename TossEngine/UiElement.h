#pragma once
#include "Renderer.h"


enum TOSSENGINE_API AnchorPoint
{
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    Center,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

// yes this works but annoying to define the enum twice, this is so that we can stringize the enum
BOOST_DESCRIBE_ENUM(AnchorPoint,
    TopLeft, TopCenter, TopRight,
    MiddleLeft, Center, MiddleRight,
    BottomLeft, BottomCenter, BottomRight)

class TOSSENGINE_API UiElement : public Renderer
{
public:
    virtual void OnInspectorGUI() override
    {
        Renderer::OnInspectorGUI();

        EnumDropdownField("Pivot Point", m_pivotPoint);
        EnumDropdownField("Anchor Point", m_anchorPoint);
    }
    

protected:
    AnchorPoint m_pivotPoint = AnchorPoint::TopLeft;
    AnchorPoint m_anchorPoint = AnchorPoint::TopLeft;

    Vector2 GetAnchorOffset(const Vector2& screenSize, const Vector2& size, AnchorPoint anchor);

};

