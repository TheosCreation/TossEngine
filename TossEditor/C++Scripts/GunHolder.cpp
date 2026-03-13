#include "GunHolder.h"
#include "Gun.h"
#include "UiManager.h"

void GunHolder::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    GameObjectAssignableField(m_gunObject, "Gun");
    FloatSliderField("Aiming Speed", m_aimingSpeed);
}

void GunHolder::onStart()
{
    if (m_gunObject)
    {
        m_currentHeldGun = m_gunObject->getComponent<Gun>();
    }
}

void GunHolder::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();

    if (inputManager.isMouseDown(MouseButtonLeft))
    {
        m_currentHeldGun->SetFiring(true);
    }
    else
    {
        m_currentHeldGun->SetFiring(false);
    }

    if (inputManager.isMouseDown(MouseButtonRight))
    {
        UiManager::Get()->SetCrosshair(false);
        m_currentHeldGun->SetAiming(true);
    }
    else
    {
        UiManager::Get()->SetCrosshair(true);
        m_currentHeldGun->SetAiming(false);
    }

}
