#pragma once
#include <TossEngine.h>

class Gun;

class GunHolder : public Component 
{
public:
    void OnInspectorGUI() override;
    void onStart() override;
    void onUpdate() override;
private:
    GameObject* m_idlePosition = nullptr;
    GameObject* m_aimingPosition = nullptr;

    GameObject* m_gunObject = nullptr;
    float m_aimingSpeed = 5.0f;

    Gun* m_currentHeldGun = nullptr;

    SERIALIZABLE_MEMBERS(m_idlePosition, m_aimingPosition, m_gunObject, m_aimingSpeed)
};

REGISTER_COMPONENT(GunHolder);