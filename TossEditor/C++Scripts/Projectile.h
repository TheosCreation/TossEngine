#pragma once
#include <TossEngine.h>

class Projectile : public Component
{
public:
    void OnInspectorGUI() override;
    void onStart() override;
    void onTriggerEnter(Collider* other) override;
private:
    float m_projectileSpeed = 20.0f;
    int m_damage = 20;
    Rigidbody* m_rigidBody = nullptr;

    SERIALIZABLE_MEMBERS(m_projectileSpeed, m_damage)
};

REGISTER_COMPONENT(Projectile);
