#pragma once
#include "Component.h"

class TOSSENGINE_API DirectionalLight : public Component
{
public:
    DirectionalLight() = default;
    ~DirectionalLight() = default;

    // Serialize 
    virtual json serialize() const override;

    // Deserialize
    virtual void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override;

    virtual void onCreate() override;
    virtual void onUpdate() override;
    virtual void onUpdateInternal() override;

    void SetIntencity(float intencity);
    void SetColor(Vector3 color);
private:
    float m_intencity = 1.0f;
    Vector3 m_color = Color::White;

    uint m_lightId = 0;
};

REGISTER_COMPONENT(DirectionalLight);
