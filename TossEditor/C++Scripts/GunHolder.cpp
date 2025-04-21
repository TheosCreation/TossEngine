#include "GunHolder.h"
#include "Gun.h"

void GunHolder::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    GameObjectAssignableField(m_idlePosition, "Idle Gun Position");
    GameObjectAssignableField(m_aimingPosition, "Aiming Gun Position");
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
        m_currentHeldGun->SetAiming(true);
    }
    else
    {
        m_currentHeldGun->SetAiming(false);
    }

    if (m_currentHeldGun->GetAiming())
    {
        Transform& gunTransform = m_currentHeldGun->getTransform();
        gunTransform.localPosition = Vector3::Lerp(gunTransform.localPosition, m_aimingPosition->m_transform.localPosition, Time::DeltaTime * m_aimingSpeed);
    }
    else
    {
        Transform& gunTransform = m_currentHeldGun->getTransform();
        gunTransform.localPosition = Vector3::Lerp(gunTransform.localPosition, m_idlePosition->m_transform.localPosition, Time::DeltaTime * m_aimingSpeed);
    }
}
