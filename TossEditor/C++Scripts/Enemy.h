#pragma once
#include <TossEngine.h>

class PlayerController;

class Enemy : public Component
{
public:
    void onStart() override;
    void OnInspectorGUI() override;
    void onUpdate() override;

    void TakeDamage(int damage);

private:
    int m_health = 100;
    float m_speed = 20.0f;

    PlayerController* m_target = nullptr;
    Rigidbody* m_rigidbody = nullptr;

    SERIALIZABLE_MEMBERS(m_health, m_speed)
};

REGISTER_COMPONENT(Enemy);
