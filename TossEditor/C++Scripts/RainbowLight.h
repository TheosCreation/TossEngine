#pragma once
#include <TossEngine.h>

class RainbowLight : public Component 
{
public:
    void OnInspectorGUI() override;
    void onStart() override;
    void onUpdate() override;
private:
    float m_speed = 2.0f;
    // TODO: only directional light for now as im going to make a Light class, and add in getComponent to cast to Light and return Directional lights, point lights and spotlights 
    DirectionalLight* m_directionalLight = nullptr;
    size_t m_colorIndex = 0;
    float m_lerpT = 0.0f;

    SERIALIZABLE_MEMBERS(m_speed)
};

REGISTER_COMPONENT(RainbowLight);
