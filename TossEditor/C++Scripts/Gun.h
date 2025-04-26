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
    GameObjectPtr m_muzzlePosition = nullptr;
    SoundPtr fireSound = nullptr;
    int m_ammoReserve = 120;
    int m_magSize = 30;


    Camera* playerCamera = nullptr;
    float shootTimer = 0.0f;
    bool m_isFiring = false;
    bool m_isAiming = false;
    int m_ammoLeft = 0;

    SERIALIZABLE_MEMBERS(m_fireRate, m_projectile, m_muzzlePosition, fireSound, m_ammoReserve, m_magSize)
};

REGISTER_COMPONENT(Gun)
