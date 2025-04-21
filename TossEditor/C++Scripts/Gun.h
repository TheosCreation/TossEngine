#pragma once
#include <TossEngine.h>

class Gun : public Component
{
public:
    void OnInspectorGUI() override;

    void onStart() override;
    void onUpdate() override;

    void SetFiring(bool firing);
    bool GetAiming() const;
    void SetAiming(bool aiming);

private:
    float m_fireRate = 0.1f;
    PrefabPtr m_projectile = nullptr;
    GameObject* m_muzzlePosition = nullptr;
    SoundPtr fireSound = nullptr;

    Camera* playerCamera = nullptr;
    float shootTimer = 0.0f;
    bool m_isFiring = false;
    bool m_isAiming = false;

    SERIALIZABLE_MEMBERS(m_fireRate, m_projectile, m_muzzlePosition, fireSound)
};

REGISTER_COMPONENT(Gun)
