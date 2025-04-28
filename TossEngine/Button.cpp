#include "Button.h"
#include "GameObject.h"
#include "Image.h"
#include "InputManager.h"

void Button::onCreateLate()
{
    // Button will require a Image component
    m_graphic = m_owner->getComponent<Image>();
    if (m_graphic == nullptr)
    {
        m_graphic = m_owner->addComponent<Image>();
    }
}

void Button::onStart()
{
    m_graphic = m_owner->getComponent<Image>();
    if (!m_graphic) Debug::LogError("Button requires a image component to function correctly");
}

void Button::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();
    Vector2 mousePos = inputManager.getMousePosition();
    //Debug::Log(mousePos);

    Rect buttonRect = m_graphic->getWorldRect();

    m_isHovered = buttonRect.Contains(mousePos);

    // Trigger onEnter event
    if (m_isHovered && !m_wasHovered && onEnter)
    {
        onEnter();
    }

    // Trigger onExit event
    if (!m_isHovered && m_wasHovered && onExit)
    {
        onExit();
    }

    // Check for click
    bool isPressed = inputManager.isMouseDown(MouseButton::MouseButtonLeft);
    if (m_isHovered && isPressed && !m_wasPressed && onClick)
    {
        Debug::Log("Button Pressed");
        onClick();
    }

    // Update previous states
    m_wasHovered = m_isHovered;
    m_wasPressed = isPressed;
}
