#pragma once
#include <TossEngine.h>

class Enemy : public Component
{
public:
    void OnInspectorGUI() override;
    void TakeDamage(int damage);

private:
    int m_health = 100;

    SERIALIZE_COMPONENT_FIELDS(m_health)
};

REGISTER_COMPONENT(Enemy);
