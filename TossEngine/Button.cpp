#include "Button.h"
#include "GameObject.h"
#include "Image.h"
#include "InputManager.h"
//#include "TossEngine.h"

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
    Debug::Log(mousePos);


    Rect buttonRect = getScreenRect(); // <-- now screen space

    //Debug::Log(buttonRect.ToString());
    m_isHovered = buttonRect.Contains(mousePos);

    if (m_isHovered && !m_wasHovered && onEnter)
    {
        Debug::Log("Mouse Entered");
        onEnter();
    }
    if (!m_isHovered && m_wasHovered && onExit)
    {
        Debug::Log("Mouse Exited");
        onExit();
    }

    bool isPressed = inputManager.isMouseDown(MouseButton::MouseButtonLeft);
    if (m_isHovered && isPressed && !m_wasPressed && onClick)
    {
        Debug::Log("Button Pressed");
        onClick();
    }

    m_wasHovered = m_isHovered;
    m_wasPressed = isPressed;
}

Rect Button::getScreenRect() const
{
    Rect worldRect = m_graphic->getWorldRect();
//
//auto& inputManager = InputManager::GetInstance();
//Rect viewport = inputManager.getViewport();
//
//Vector2 windowSize = TossEngine::GetInstance().GetWindow()->getInnerSize();
//
//float scaleX = viewport.width / windowSize.x;
//float scaleY = viewport.height / windowSize.y;
//
    // Now remap
    //Rect screenRect;
    //screenRect.left = viewport.left + worldRect.left * scaleX;
    //screenRect.top = viewport.top + worldRect.top * scaleY;
    //screenRect.width = viewport.left + worldRect.width * scaleX;
    //screenRect.height = viewport.top + worldRect.height * scaleY;

    return worldRect;
}
