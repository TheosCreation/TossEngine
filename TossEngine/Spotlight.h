#pragma once
#include "Component.h"
#include "LightManager.h"

class TOSSENGINE_API Spotlight : public Component
{
public:
    Spotlight() = default;
    ~Spotlight() = default;

    virtual void OnInspectorGUI() override;

    void onCreateLate() override;
    virtual void onUpdate() override;
    virtual void onUpdateInternal() override;
    void OnDrawGizmosSelected(UniformData data) override;
    void onDestroy() override;


    void SetIntencity(float intencity);
    void SetColor(Vector3 color);
    void SetCutoff(float newCutoff);
    void SetOuterCutoff(float newOuterCutoff);
    void SetRange(float range);
private:
    float m_intencity = 1.0f;
    Vector3 m_color = Color::White;
    float m_cutoff = 25.0f;
    float m_outerCutoff = 35.0f;
    float m_range = 10.0f;

    uint m_lightId = 0;

    SERIALIZABLE_MEMBERS(m_intencity, m_color, m_cutoff, m_outerCutoff, m_range)
};

REGISTER_COMPONENT(Spotlight);
