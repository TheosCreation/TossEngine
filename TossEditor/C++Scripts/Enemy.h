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

    PlayerController* m_target = nullptr;
    Rigidbody* m_rigidbody = nullptr;

    vector<std::string> m_layerNames;

    SERIALIZABLE_MEMBERS(m_health, m_speed, m_layerNames, m_wallDetectDistance)
};

REGISTER_COMPONENT(Enemy);
