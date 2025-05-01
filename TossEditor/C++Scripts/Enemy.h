#pragma once
#include <TossEngine.h>

class PlayerController;

class Enemy : public Component
{
public:
    void onStart() override;
    void OnInspectorGUI() override;
    void onFixedUpdate() override;

    void TakeDamage(int damage);

private:
    int m_health = 100;
    float m_speed = 20.0f;
    float m_wallDetectDistance = 5.0f;
    float m_attackDistance = 3.0f;
    int m_attackDamage = 20;
    float m_attackCoolDown = 1.0f;


    PlayerController* m_target = nullptr;
    Rigidbody* m_rigidbody = nullptr;
    float m_attackCooldownTimer = 0.0f;


    SERIALIZABLE_MEMBERS(m_health, m_speed, m_wallDetectDistance, m_attackDistance, m_attackDamage, m_attackDistance)
};

REGISTER_COMPONENT(Enemy);
