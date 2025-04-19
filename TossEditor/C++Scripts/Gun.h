#pragma once
#include <TossEngine.h>

class Gun : public Component
{
public:
    void OnInspectorGUI() override;

    void onStart() override;
    void onUpdate() override;

private:
    float m_fireRate = 0.1f;
    PrefabPtr m_projectile = nullptr;
    GameObject* m_muzzlePosition = nullptr;

    Camera* playerCamera = nullptr;
    float shootTimer = 0.0f;

    SERIALIZABLE_MEMBERS(m_fireRate, m_projectile, m_muzzlePosition)
};

REGISTER_COMPONENT(Gun)
