#include "Gun.h"

void Gun::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("FireRate", m_fireRate);
    ResourceAssignableField(m_projectile, "Projectile");
    ResourceAssignableField(fireSound, "Shoot Sound");
    GameObjectAssignableField(m_muzzlePosition, "Muzzle Position");
}

void Gun::onStart()
{
    playerCamera = m_owner->getComponentInParent<Camera>();
}

void Gun::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();
    if(inputManager.isMouseDown(MouseButtonLeft) && shootTimer <= 0.0f)
    {
        shootTimer = m_fireRate;

        Transform& camTransform = playerCamera->getTransform();
        if (m_projectile)
        {
            AudioEngine::GetInstance().playSound(fireSound);
            GameObject* spawnedProjectile = m_owner->getGameObjectManager()->Instantiate(m_projectile, m_muzzlePosition->m_transform.position, camTransform.rotation);
            
        }
    }
    else
    {
        shootTimer -= Time::DeltaTime;
    }
}
