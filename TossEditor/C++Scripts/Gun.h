#pragma once
#include "TossEngine.h"

class Gun : public Component
{
public:
    json serialize() const override;
    void deserialize(const json& data) override;
    void OnInspectorGUI() override;
    void onUpdate() override;

private:
    //float fireRate = 0.1f;
    PrefabPtr m_projectile = nullptr;
};

REGISTER_COMPONENT(Gun);
