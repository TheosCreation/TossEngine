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

        if (EnumDropdownField("Pivot Point", m_pivotPoint))
        {
            UpdatePivotOffset();
        }
        EnumDropdownField("Anchor Point", m_anchorPoint);
    }

    virtual void UpdateSize() {} //will be defined differently for each type inherited from this
    

protected:
    AnchorPoint m_pivotPoint = AnchorPoint::TopLeft;
    AnchorPoint m_anchorPoint = AnchorPoint::TopLeft;


    Vector2 m_pivotOffset;
    Vector2 m_size;

    Vector2 GetAnchorOffset(const Vector2& screenSize, const Vector2& size, AnchorPoint anchor) const;

    void UpdatePivotOffset();

};

