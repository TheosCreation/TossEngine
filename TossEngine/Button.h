#pragma once
#include "Component.h"

class Image;

class TOSSENGINE_API Button : public Component
{
public:
    void onCreateLate() override;
    void onStart() override;
    void onUpdate() override;

    Rect getScreenRect() const;

    std::function<void()> onClick;
    std::function<void()> onEnter;
    std::function<void()> onExit;

private:
    Image* m_graphic = nullptr;

    bool m_isHovered = false;
    bool m_wasHovered = false;
    bool m_wasPressed = false;
};

REGISTER_COMPONENT(Button);
