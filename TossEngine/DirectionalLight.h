#pragma once
#include "Component.h"

class TOSSENGINE_API DirectionalLight : public Component
{
public:
    DirectionalLight() = default;
    ~DirectionalLight() = default;
    
    void OnInspectorGUI() override;

    void onCreateLate() override;
    void onUpdate() override;
    void onUpdateInternal() override;
    void onDestroy() override;
    void OnDrawGizmosSelected(UniformData data) override;

    void SetIntencity(float intencity);
    void SetColor(Vector3 color);
private:
    float m_intencity = 1.0f;
    Vector3 m_color = Color::White;

    uint m_lightId = 0;

    SERIALIZABLE_MEMBERS(m_intencity, m_color)
};

REGISTER_COMPONENT(DirectionalLight)