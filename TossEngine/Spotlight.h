#pragma once
#include "Component.h"
#include "LightManager.h"

class TOSSENGINE_API Spotlight : public Component
{
public:
    Spotlight() = default;
    ~Spotlight() = default;

    // Serialize 
    virtual json serialize() const override;

    // Deserialize
    virtual void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override;

    virtual void onCreate() override;
    virtual void onUpdate(float deltaTime) override;
    virtual void onUpdateInternal() override;


    void SetIntencity(float intencity);
    void SetColor(Vector3 color);
    void SetCutoff(float newCutoff);
    void SetOuterCutoff(float newOuterCutoff);
private:
    float m_intencity = 1.0f;
    Vector3 m_color = Color::White;
    float m_cutoff = 25.0f;
    float m_outerCutoff = 35.0f;

    uint m_lightId = 0;
};

REGISTER_COMPONENT(Spotlight);
