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
    //Rect buttonRect = getScreenRect();
    //Debug::Log(buttonRect.ToString());
}

void Button::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();
    Vector2 mousePos = inputManager.getMousePosition();

    Scene* scene = m_owner->getGameObjectManager()->getScene();
    Vector2 viewportPosition = scene->getPosition();
    //Debug::Log(mousePos);


    Rect buttonRect = getScreenRect(); // <-- now screen space

    //Debug::Log(buttonRect.ToString());
    m_isHovered = buttonRect.Contains(mousePos);

    if (m_isHovered && !m_wasHovered)
    {
        //Debug::Log("Mouse Entered");

        if (onEnter) onEnter();
    }
    if (!m_isHovered && m_wasHovered)
    {
        //Debug::Log("Mouse Exited");
        if (onExit) onExit();
    }

    bool isPressed = inputManager.isMousePressed(MouseButton::MouseButtonLeft, false);
    if (m_isHovered && isPressed && !m_wasPressed)
    {
        //Debug::Log("Button Pressed");
        if (onClick) onClick();
    }

    m_wasHovered = m_isHovered;
    m_wasPressed = isPressed;
}

Rect Button::getScreenRect() const
{
    Scene* scene = m_owner->getGameObjectManager()->getScene();
    Vector2 viewportPosition = scene->getPosition(); 
    Vector2 viewportSize = scene->getSize();         

     //Draw viewport outline (green)
    //ImGui::GetForegroundDrawList()->AddRect(
    //    ImVec2(viewportPosition.x, viewportPosition.y),
    //    ImVec2(viewportPosition.x + viewportSize.x, viewportPosition.y + viewportSize.y),
    //    IM_COL32(0, 255, 0, 255),
    //    0.0f,
    //    0,
    //    2.0f
    //);

    Rect worldRect = m_graphic->getWorldRect();

    Rect screenRect;
    screenRect.left = viewportPosition.x + worldRect.left;
    screenRect.top = viewportPosition.y - worldRect.top;

    screenRect.width = worldRect.width;
    screenRect.height = worldRect.height;

    
    //ImGui::GetForegroundDrawList()->AddRect(
    //    ImVec2(screenRect.left, screenRect.top),
    //    ImVec2(screenRect.left + screenRect.width, screenRect.top + screenRect.height),
    //    IM_COL32(255, 0, 0, 255),
    //    0.0f,
    //    0,
    //    2.0f
    //);

    return screenRect;
}