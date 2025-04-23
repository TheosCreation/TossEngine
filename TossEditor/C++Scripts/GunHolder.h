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
    GameObjectPtr m_idlePosition = nullptr;
    GameObjectPtr m_aimingPosition = nullptr;

    GameObjectPtr m_gunObject = nullptr;
    float m_aimingSpeed = 5.0f;

    Gun* m_currentHeldGun = nullptr;

    SERIALIZABLE_MEMBERS(m_idlePosition, m_aimingPosition, m_gunObject, m_aimingSpeed)
};

REGISTER_COMPONENT(GunHolder);