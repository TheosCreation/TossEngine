#pragma once
#include <TossEngine.h>

class PlayerController;

class Enemy : public Component
{
public:
    void onStart() override;
    void OnInspectorGUI() override;
    void TakeDamage(int damage);

private:
    int m_health = 100;

    PlayerController* m_target = nullptr;

    SERIALIZABLE_MEMBERS(m_health)
};

REGISTER_COMPONENT(Enemy);
