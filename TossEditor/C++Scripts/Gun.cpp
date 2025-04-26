#include "Gun.h"
#include "UiManager.h"

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
    if(m_isFiring && shootTimer <= 0.0f)
    {
        shootTimer = m_fireRate;

        Transform& camTransform = playerCamera->getTransform();
        if (m_projectile)
        {
            AudioEngine::GetInstance().playSound(fireSound);
            GameObjectPtr spawnedProjectile = m_owner->getGameObjectManager()->Instantiate(m_projectile, m_muzzlePosition->m_transform.position, camTransform.rotation);
            
        }
    }
    else
    {
        shootTimer -= Time::DeltaTime;
    }
}

void Gun::SetFiring(bool firing)
{
    m_isFiring = firing;
}

bool Gun::GetAiming() const
{
    return m_isAiming;
}

void Gun::SetAiming(bool aiming)
{
    m_isAiming = aiming;
}
