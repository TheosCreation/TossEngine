#pragma once
#include <TossEngine.h>

class Enemy : public Component
{
public:
    void OnInspectorGUI() override;
    void TakeDamage(int damage);

private:
    int m_health = 100;
    Vector3 vector;

    SERIALIZABLE_MEMBERS(m_health, vector)
};

REGISTER_COMPONENT(Enemy);
