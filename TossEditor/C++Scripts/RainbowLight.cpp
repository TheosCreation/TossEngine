#include "RainbowLight.h"

void RainbowLight::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    FloatSliderField("Speed", m_speed);
}

void RainbowLight::onStart()
{
    m_directionalLight = m_owner->getComponent<DirectionalLight>();
}

void RainbowLight::onUpdate()
{
    m_lerpT += m_speed * Time::DeltaTime;
    if (m_lerpT >= 1.0f) {
        m_lerpT -= 1.0f;
        m_colorIndex = (m_colorIndex + 1) % Color::Rainbow.size();
    }

    // next index in the ring
    size_t nextIndex = (m_colorIndex + 1) % Color::Rainbow.size();

    // compute and apply the color
    if (m_directionalLight)
        m_directionalLight->SetColor(Vector3::Lerp(Color::Rainbow[m_colorIndex], Color::Rainbow[nextIndex], m_lerpT));
}
