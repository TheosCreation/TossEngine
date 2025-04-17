#pragma once
#include <TossEngine.h>

class Projectile : public Component
{
public:
    void OnInspectorGUI() override;
    json serialize() const override;
    void deserialize(const json& data) override;
    void onStart() override;
    void onTriggerEnter(Collider* other) override;
private:
    float m_projectileSpeed = 20.0f;
    int m_damage = 20;
    Rigidbody* m_rigidBody = nullptr;
};

REGISTER_COMPONENT(Projectile);
